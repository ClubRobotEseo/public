--2017_06_09	Nirgal
-- Club robotique de l'ESEO

library ieee;
use ieee.std_logic_1164.all;
use work.VGA_pkg.all;
use work.SegmentDisplay_pkg.all;

entity Top is 

	port(	
		clk, reset : in std_logic;
		
		-- VGA
		hsync_n, vsync_n : out std_logic;
		vgaRed : out std_logic_vector(2 downto 0);
		vgaGreen : out std_logic_vector(2 downto 0);
		vgaBlue : out std_logic_vector(2 downto 1);
		
		-- Mémoire SRAM
		memWait_n : in std_logic;
		memOE_n : out std_logic;
		memWR_n : out std_logic;
		memAdv_n : out std_logic;
		memClk : out std_logic;
		memAdr : out std_logic_vector(26 downto 1);
		memData : inout std_logic_vector(15 downto 0);

		ramCS_n : out std_logic;
		ramCRE : out std_logic;
		ramUB_n : out std_logic;
		ramLB_n : out std_logic;
		
		-- Bus de communication parallèle
		parallel_datas : in std_logic_vector(15 downto 0);
		parallel_clk : in std_logic;
		parallel_cs : in std_logic;
		
		-- IHM
		sw : in std_logic_vector(7 downto 0);
		dispSeg_n : out std_logic_vector(0 to 7);
		dispAn_n : out std_logic_vector(3 downto 0)
	);

end Top;

architecture Structural of Top is--Déclaration de l'architecture correspondant à l'entité.
	signal end_of_pixel : std_logic;
	signal blanking : std_logic;
	signal x : integer range 0 to VGA_X_MAX;
	signal y : integer range 0 to VGA_Y_MAX;

	signal pixel : VGAColor_t;							-- pixel courant
	signal full_screen_array : full_screen_array_t;		--buffer complet de l'image !

	signal we_can_write : std_logic;
	signal ask_for_write : std_logic;
	signal pixels_to_write : std_logic_vector(15 downto 0);
	signal write_x : integer range 0 to VGA_SCREEN_WIDTH-1;
	signal write_y : integer range 0 to VGA_SCREEN_HEIGHT-1;
	
	signal digits : SegmentDisplayDigitArray_t;
begin 

	VGAController_inst : entity work.VGAController(Behavioral)
		port map(
			clk => clk,
			reset => reset,
			x => x,
			y => y,
			hsync_n => hsync_n,
			vsync_n => vsync_n,
			blanking => blanking,
			end_of_pixel => end_of_pixel
		);
 
	
	--vgaRed <= pixel.red;
	--vgaGreen <= pixel.green;
	--vgaBlue <= pixel.blue;
	p_vga : process (blanking, sw, pixel)
	begin
	if blanking = '1' then
		vgaRed <= "000";
		vgaGreen <= "000";
		vgaBlue <= "00";
	else
		if sw(7) = '1' then
			vgaRed <= sw(6 downto 5) & "0";
			vgaGreen <= sw(4 downto 2);
			vgaBlue <= sw(1 downto 0);
		else
			vgaRed <= pixel.red;
			vgaGreen <= pixel.green;
			vgaBlue <= pixel.blue;
		end if;
		
	end if;
	end process p_vga;
	
		
	SRAM_inst : entity work.SRAM(Behavioral)
	port map(
		x=>x,
		y=>y,
		clk => clk,
		reset => reset,
		blanking=>blanking,
		memAdr => memAdr,
		memAdv_n => memAdv_n,
		memClk => memClk,
		memWait_n => memWait_n,
		memOE_n => memOE_n,
		memWR_n => memWR_n,
		memData => memData,
		ramCs_n => ramCs_n,
		ramUB_n => ramUB_n,
		ramLB_n => ramLB_n,
		ramCRE => ramCRE,
		pixel => pixel,
		ask_for_write => ask_for_write,
		pixels_to_write => pixels_to_write,
		write_x => write_x,
		write_y => write_y
	);
	
	parallel_inst : entity work.parallel(Behavioral)
	port map(
		reset => reset,
		clk => clk,
		we_can_write => we_can_write,
		parallel_datas => parallel_datas,
		parallel_clk => parallel_clk,
		parallel_cs => parallel_cs,
		ask_for_write => ask_for_write,
		pixels_to_write => pixels_to_write,
		write_x => write_x,
		write_y => write_y
	);
	
	we_can_write <= '1' when x >= VGA_SCREEN_WIDTH - 2 and x < VGA_X_MAX-1
						else '1' when y >= VGA_SCREEN_HEIGHT and x < VGA_X_MAX-1
						else '0';

	segments_display_inst : entity work.SegmentDisplayController(Comportementale) 
	generic map(
		-- La fréquence d'horloge du FPGA	
		CLK_FREQUENCY_HZ => 100_000_000
	)
    port map(
		clk => clk,
		anodes_n => dispAn_n,
		segments_n => dispSeg_n,
		digits => digits
    );

	digits(0) <= write_x mod 10;
	digits(1) <= write_x/10 mod 10;
	digits(2) <= write_x/100 mod 10;
	digits(3) <= write_x/1000 mod 10;

end Structural;