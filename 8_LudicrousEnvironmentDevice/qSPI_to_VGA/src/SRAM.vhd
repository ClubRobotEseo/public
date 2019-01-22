library ieee;
use ieee.std_logic_1164.all;
use work.VGA_pkg.all;
use ieee.std_logic_arith.ALL;

entity SRAM is
	port(
		clk : in std_logic;
		reset : in std_logic;
		memAdv_n, memClk, memOE_n, memWR_n, ramCS_n, ramUB_n, ramLB_n, ramCRE : out std_logic;
		memWait_n : in std_logic;
		memData : inout std_logic_vector(15 downto 0);
		memAdr : out std_logic_vector (26 downto 1);
		blanking : in std_logic;
		x : in integer range 0 to VGA_X_MAX;
		y : in integer range 0 to VGA_Y_MAX;
		pixel : out VGAColor_t;
		
		-- Demande d'accès mémoire
		ask_for_write : in std_logic;
		pixels_to_write : in std_logic_vector(15 downto 0);
		write_x : in integer range 0 to VGA_SCREEN_WIDTH-1;
		write_y : in integer range 0 to VGA_SCREEN_HEIGHT-1
	);

end SRAM;

architecture Behavioral of SRAM is

	constant WRITE_DURATION : integer := 7;				-- en nombre de fronts de clk !
	constant CYCLE_PER_DOUBLE_PIXEL : integer := 8; 	-- en nombre de fronts de clk !
	
	type State_t is(IDLE, WRITING, READING);
	signal state_reg : State_t;

	signal blanking_reg : std_logic;
	signal read_counter : natural range 0 to CYCLE_PER_DOUBLE_PIXEL-1;
	signal write_counter : integer range 0 to WRITE_DURATION;
	signal memadr_reg : std_logic_vector (26 downto 1);
	
	-- gestion de l'écriture
  signal pixels_to_write_reg : std_logic_vector(15 downto 0);
  signal	write_x_reg : integer range 0 to VGA_SCREEN_WIDTH-1;
  signal write_y_reg : integer range 0 to VGA_SCREEN_HEIGHT-1;
  
  --gestion de la lecture
  signal pixels_read : std_logic_vector(15 downto 0);
	
begin

	memAdv_n <= '0';
	memClk <= '0';
	ramCS_n <= '0';
	ramCRE <= '0';
	
	ramUB_n <= '0';
	ramLB_n <= '0';

	blanking_reg <= blanking;
	

	memWR_n <= '0' when state_reg = WRITING else '1';	 -- active l'écriture à l'état bas
	memOE_n <= '0' when state_reg = READING else '1';	 -- active la lecture à l'état bas

	p_state_reg : process(clk,reset) 
	begin
	if reset = '1'
		then state_reg <= IDLE;
	elsif rising_edge(clk) then
		case state_reg is
		  when IDLE =>
			state_reg <= READING;
		  when WRITING =>
			if write_counter = WRITE_DURATION then
				state_reg <= READING; 
			end if;
		  when READING =>
			if ask_for_write = '1' then
				state_reg <= WRITING; 
			end if;
		end case;
		end if;
	end process p_state_reg;
	
	
		
-------------------------------------ADRESSE DU PIXEL----------------------------------------------
-- l'adresse d'un pixel en cours de lecture dépend de x et y si blanking = '0'
-- Sinon, -> lecture de l'adresse du premier pixel de la ligne suivante.
---------------------------------------------------------------------------------------------------

	memAdr <= conv_std_logic_vector((write_y_reg*VGA_SCREEN_WIDTH+write_x_reg)/2,26) when state_reg = WRITING
			else conv_std_logic_vector(((y*VGA_SCREEN_WIDTH)+x+2)/2, 26) when state_reg = READING and blanking_reg = '0'
		else  conv_std_logic_vector(((y+1)*VGA_SCREEN_WIDTH)/2, 26) when state_reg = READING and blanking_reg = '1'
		else "00000000000000000000000000";

------------------------------------- PROCESS PIXEL_REG -------------------------------------------	
-- Le port de sortie pixel est de type VGAColor_t. Il existe donc 3 sorties :		  
-- pixel.red, pixel.green et pixel.blue.											
--																					
-- Le portie de sortie memData est codé sur 2 octets, ce qui correspond à 2 pixels. 
-- intérêt : le temps de lecture de la RAM est 70ns, celui d'un pixel de 40ns... il faut donc impérativement lire 2 pixels d'un coup !
----------------------------------------------------------------------------------------------------


--ATTENTION, si une écriture survient alors qu'on souhaitait lire, le pixel courant est maintenu à la place de celui qu'on a pas le temps de lire !

--Lecture des données
p_pixels_read : process (clk)
begin
	if(rising_edge(clk)) then
		if read_counter = CYCLE_PER_DOUBLE_PIXEL-1 and state_reg = READING then
			pixels_read <= memData;
		end if;
	end if;
end process p_pixels_read;

p_pixel : process (pixels_read, blanking_reg, x)
begin
	if blanking_reg = '1' then
		pixel <= VGA_COLOR_BLACK;
	elsif x mod 2 = 1	then
		pixel.red <= pixels_read (15 downto 13);
		pixel.green <= pixels_read (12 downto 10);	   
		pixel.blue <= pixels_read (9 downto 8);
	else
		pixel.red <= pixels_read (7 downto 5);
		pixel.green <= pixels_read (4 downto 2); 
		pixel.blue <= pixels_read (1 downto 0);	
	end if;
end process p_pixel;


--Ecriture des données
memData <= pixels_to_write_reg when state_reg = WRITING else "ZZZZZZZZZZZZZZZZ";	
	
	

--------------------------------------- PROCESS read_counter -------------------------------------
-- Ce processus réalise un read_counter de 0 à 8. Il permet de compter 80ns, en effet cela 
-- correspond au temps nécessaire pour effectuer la lecture de 2 pixels.
----------------------------------------------------------------------------------------------
p_read_counter : process(clk, reset)
begin
	if reset = '1' then
		read_counter <=0;
	elsif rising_edge(clk) then
		if read_counter = CYCLE_PER_DOUBLE_PIXEL-1 then
			read_counter <= 0;
		else
			read_counter <= read_counter + 1;
		end if;
	end if;
end process p_read_counter;


-- on mémorise tout nouvel ordre d'écriture !

p_buffer_write_order : process(clk, reset)
begin
	if reset = '1' then
		pixels_to_write_reg <= "0000000000000000";
		write_x_reg <= 0;
		write_y_reg <= 0;
	elsif rising_edge(clk) then
		if ask_for_write = '1' then
			pixels_to_write_reg <= pixels_to_write;
			write_x_reg <= write_x;
			write_y_reg <= write_y;
			
		end if;
	end if;
end process p_buffer_write_order;


p_write_counter : process(clk, reset)
begin
	if reset = '1' then
		write_counter <= 0;	
	elsif rising_edge(clk) then
		if state_reg = WRITING then
			if write_counter = WRITE_DURATION then
				write_counter <= 0;
			else
				write_counter <= write_counter + 1;
			end if;
		else
			write_counter <= 0;
		end if;
	end if;
end process p_write_counter;



end Behavioral;