library ieee;
use ieee.std_logic_1164.all;
use work.VGA_pkg.all;
use ieee.std_logic_arith.ALL;

entity parallel is
	port(
		clk : in std_logic;
		reset : in std_logic;
		
		-- Bus de communication parallèle
		parallel_datas : in std_logic_vector(15 downto 0);
		parallel_clk : in std_logic;
		parallel_cs : in std_logic;
		
		-- Demande d'accès mémoire
		we_can_write : in std_logic;
		ask_for_write : out std_logic;
		pixels_to_write : out std_logic_vector(15 downto 0);
		write_x : out integer range 0 to VGA_SCREEN_WIDTH-1;
		write_y : out integer range 0 to VGA_SCREEN_HEIGHT-1
	);

end parallel;

architecture Behavioral of parallel is
	signal x_reg, x1, x2 : integer range 0 to VGA_SCREEN_WIDTH-1;
	signal y_reg, y1, y2 : integer range 0 to VGA_SCREEN_HEIGHT-1;
	type State_t is (IDLE, WAIT_CS, WAIT_COMMAND, WAIT_FOR_X1, WAIT_FOR_X2, WAIT_FOR_Y1, WAIT_FOR_Y2, WAIT_FOR_DATA, WAIT_FOR_CS_RISE);
	constant COMMAND_PAINT_RECTANGLE : std_logic_vector(15 downto 0) := x"0000";
	signal state : State_t;
	signal rising_clk : std_logic;
	signal synchro_clk : std_logic_vector(1 downto 0);
	signal ask_for_write_reg : std_logic;
	signal parallel_datas_integer : integer range 0 to 65535;
	constant BUF_SIZE : integer := 200;
	type buf_x_array_t is array(BUF_SIZE-1 downto 0) of integer range 0 to VGA_SCREEN_WIDTH-1;
	type buf_y_array_t is array(BUF_SIZE-1 downto 0) of integer range 0 to VGA_SCREEN_HEIGHT-1;
	type buf_datas_array_t is array(BUF_SIZE-1 downto 0) of std_logic_vector(15 downto 0);
	
	signal pixels_to_write_reg : std_logic_vector(15 downto 0);
	signal buf_x : buf_x_array_t;
	signal buf_y : buf_y_array_t;
	signal buf_datas : buf_datas_array_t;
	signal buf_write_index : integer range 0 to BUF_SIZE - 1;
	signal buf_read_index : integer range 0 to BUF_SIZE - 1;
	signal write_count : integer range 0 to 7;
	
begin

	-- state machine
	p_state : process(clk,reset)
	begin
		if reset = '1' then
			state <= IDLE;
		elsif rising_edge(clk) then
			if parallel_cs = '1' then
				state <= WAIT_CS;
			else
				case state is
					when IDLE =>
						state <= WAIT_CS;
					when WAIT_CS =>
						if parallel_cs = '0' then
							state <= WAIT_COMMAND;
						end if;
					when WAIT_COMMAND => 
						if rising_clk = '1' then
							if parallel_datas = COMMAND_PAINT_RECTANGLE then
								state <= WAIT_FOR_X1;
							else
								state <= WAIT_FOR_CS_RISE;
							end if;
						end if;
					when WAIT_FOR_X1 =>
						if rising_clk = '1' then
							state <= WAIT_FOR_X2;
						end if;
					when WAIT_FOR_X2 =>
						if rising_clk = '1' then
							state <= WAIT_FOR_Y1;
						end if;	
					when WAIT_FOR_Y1 =>
						if rising_clk = '1' then
							state <= WAIT_FOR_Y2;
						end if;
					when WAIT_FOR_Y2 =>
						if rising_clk = '1' then
							state <= WAIT_FOR_DATA;
						end if;
					when WAIT_FOR_DATA =>
						--on attend parallel_cs (géré en dehors de ce case)
					when WAIT_FOR_CS_RISE => -- commande non reconnue, reserved for futur use !
						
				end case;
			end if;
		end if;
	end process p_state;
	
	
	p_synchro_clk : process(clk, reset)
	begin
		if reset = '1' then
			synchro_clk <= "00";
		elsif rising_edge(clk) then

			synchro_clk(1) <= synchro_clk(0);
			synchro_clk(0) <= parallel_clk;
			if synchro_clk = "10" then 
				rising_clk <= '1';
				parallel_datas_integer <= conv_integer(unsigned(parallel_datas));
			else
				rising_clk <= '0';
			end if;
		end if;
	end process p_synchro_clk;

	
	
	p_xy_datas : process (clk, reset)
	begin
		if reset = '1' then
			x1 <= 0;
			x2 <= 0;
			y1 <= 0;
			y2 <= 0;
			x_reg <= 0;
			y_reg <= 0;
			ask_for_write_reg <= '0';
			--pixels_to_write <= "0000000000000000";
		elsif rising_edge(clk) then
			if rising_clk = '1' then
				case state is
					when WAIT_FOR_X1 =>
						if parallel_datas_integer < VGA_SCREEN_WIDTH then x1 <= parallel_datas_integer; else x1 <= 0; end if;
					when WAIT_FOR_X2 =>
						if parallel_datas_integer < VGA_SCREEN_WIDTH then x2 <= parallel_datas_integer; else x2 <= VGA_SCREEN_WIDTH-1; end if;
					when WAIT_FOR_Y1 =>
						if parallel_datas_integer < VGA_SCREEN_HEIGHT then y1 <= parallel_datas_integer; else y1 <= 0; end if;
					when WAIT_FOR_Y2 =>
						if parallel_datas_integer < VGA_SCREEN_HEIGHT then y2 <= parallel_datas_integer; else y2 <= VGA_SCREEN_HEIGHT-1; end if;
						x_reg <= x1;
						y_reg <= y1;
					when WAIT_FOR_DATA =>
						ask_for_write_reg <= '1';			
						--pixels_to_write <= parallel_datas;
						pixels_to_write_reg <= parallel_datas;
					when others =>
				end case;
			else
				ask_for_write_reg <= '0';	-- ask_for_write_reg ne peux pas être à 1 deux fois de suite !
				if ask_for_write_reg = '1' then	--on vient juste de faire une écriture il faut incrémenter x_reg et y_reg
					if (x_reg >= VGA_SCREEN_WIDTH - 1) or (x_reg >= x2) then		--dernier pixel de la ligne
						x_reg <= x1;		
						-- incrément du numéro de ligne (y)
						if (y_reg >= VGA_SCREEN_HEIGHT - 1) or (y_reg >= y2) then	--dernier pixel de l'image !
							y_reg <= y1;	--n'est pas sensé de produire sur une mise à jour d'une zone... sauf si on veut que cette zone soit écrasée en boucle ! (vidéo encapsulée... mais c'est moche)
						else
							y_reg <= y_reg + 1;	
						end if;
					else
						x_reg <= x_reg + 2;
					end if;
				end if;
			end if;
			
		end if;
	end process p_xy_datas;
	
	--write_x <= x_reg;
	--write_y <= y_reg;
	--ask_for_write <= ask_for_write_reg;
	
	p_buf : process(reset, clk)
	begin
		if reset = '1' then
			buf_write_index <= 0;
			buf_read_index <= 0;
			pixels_to_write <= "0000000000000000";
			
		elsif rising_edge(clk) then
			ask_for_write <= '0';	--hypothèse
			
		
			--bufferise
			if ask_for_write_reg = '1' then
				buf_x(buf_write_index) <= x_reg;
				buf_y(buf_write_index) <= y_reg;
				buf_datas(buf_write_index) <= pixels_to_write_reg;
				
				--incrément de l'index d'écriture
				if buf_write_index < BUF_SIZE - 1 then
					buf_write_index <= buf_write_index + 1;
				else
					buf_write_index <= 0;
				end if;
			end if;
			
			
			if we_can_write = '1' then 
				if write_count < 7 then
					write_count <= write_count + 1;
				else
					write_count <= 0;
				end if;
			
				if write_count = 0 then						
					--incrément de l'index de lecture
					if buf_read_index /= buf_write_index then
						
						-- c'est le moment de demander une écriture
						write_x <= buf_x(buf_read_index);
						write_y <= buf_y(buf_read_index);
						pixels_to_write <= buf_datas(buf_read_index);
						ask_for_write <= '1';
							
						if buf_read_index < BUF_SIZE - 1 then
							buf_read_index <= buf_read_index + 1;
						else
							buf_read_index <= 0;
						end if;
					end if;
				end if;
			else
				write_count <= 0;
			end if;
				
		end if;
	end process p_buf;
	
	
	
	
end Behavioral;