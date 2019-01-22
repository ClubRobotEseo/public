--2017_06_09	Nirgal
-- Club robotique de l'ESEO

library ieee;
use ieee.std_logic_1164.all;
use work.VGA_pkg.all;

entity VGAController is --Déclaration de l'entité gérant le port VGA.

	port(	
		clk, reset : in std_logic;
		x : out integer range 0 to VGA_X_MAX;
		y : out integer range 0 to VGA_Y_MAX;
		end_of_pixel : out std_logic;
		blanking, hsync_n, vsync_n : out std_logic
	);

end VGAController;

architecture Behavioral of VGAController is--Déclaration de l'architecture correspondant à l'entité.
	constant PREDIV_VALUE : integer := 4;
	signal prediv : integer range 0 to 3;
	signal end_of_pixel_reg: std_logic;
	signal x_reg : integer range 0 to VGA_X_MAX;
	signal y_reg : integer range 0 to VGA_Y_MAX;

begin 

	--Processus gérant l'incrémentation et le reset du prédiviseur
	p_compteur_reg : process(clk, reset)
	begin
		if reset = '1' then
			prediv <= 0;
		elsif rising_edge(clk) then 
			if end_of_pixel_reg = '1' then
				prediv <= 0;
			else 
				prediv <= prediv+1;
			end if;
		end if;
	end process p_compteur_reg;

	end_of_pixel_reg <= '1' when prediv = PREDIV_VALUE - 1 else '0';
	
	--Processus gérant le déplacement en x du " pointeur pixel"
	p_x_reg : process(clk, reset)
	begin
		if rising_edge(clk) then
			if end_of_pixel_reg = '1' then
				if x_reg < VGA_X_MAX then
					x_reg <= x_reg+1;
				else 
					x_reg <= 0;
				end if;
			end if;
		end if;
	end process p_x_reg;

	--Processus gérant le déplacement en y du " pointeur pixel
	p_y_reg : process(clk,reset)
	begin 
		if rising_edge(clk) then
			if x_reg = VGA_X_MAX and end_of_pixel_reg = '1' then
				if y_reg < VGA_Y_MAX then
					y_reg <= y_reg+1; 
				else 
					y_reg <= 0;
				end if;
			end if;
		end if;
	end process p_y_reg;

	blanking <= '1' when (x_reg >= VGA_SCREEN_WIDTH) or (y_reg >= VGA_SCREEN_HEIGHT)		else '0';
		
	hsync_n <= '0' when (x_reg >= VGA_HBL_START and x_reg <= VGA_HBL_END)		else '1';
		
	vsync_n <= '0' when (y_reg >= VGA_VBL_START and y_reg <= VGA_VBL_END)		else '1';

	end_of_pixel <= end_of_pixel_reg;

	x <= x_reg;
	y <= y_reg;
end Behavioral;

