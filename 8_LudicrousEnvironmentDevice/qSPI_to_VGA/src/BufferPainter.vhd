--2017_06_09	Nirgal
-- Club robotique de l'ESEO

library ieee;
use ieee.std_logic_1164.all;
use work.VGA_pkg.all;

entity BufferPainter is --Déclaration de l'entité gérant le port VGA.

	port(	
		clk, reset : in std_logic;
		x : in integer range 0 to VGA_SCREEN_WIDTH-1;
		y : in integer range 0 to VGA_SCREEN_HEIGHT-1;
		blanking : in std_logic;
		end_of_pixel : in std_logic;
		full_screen_array : in full_screen_array_t;
		pixel : out VGAColor_t
	);

end BufferPainter;

architecture Behavioral of BufferPainter is
	signal index : integer range 0 to VGA_PIXEL_NUMBER-1;
	signal next_x_reg : integer range 0 to VGA_X_MAX;
	signal next_y_reg : integer range 0 to VGA_Y_MAX;
begin 

	next_x_reg <= x + 1 when x < VGA_SCREEN_WIDTH else 0;
	next_y_reg <= y + 1 when x = VGA_X_MAX - 1 else y;
	
	
	--process asynchrone pour produire le signal index...
	p_index : process(next_x_reg,next_y_reg,blanking,full_screen_array)
	begin
		if blanking = '1' then
			index <= 0;
		else
			index <= next_x_reg + next_y_reg*VGA_SCREEN_HEIGHT;
		end if;
	end process p_index;
	
	next_pixel <= full_screen_array(index) when blanking = '0' 
		else VGA_COLOR_BLACK;
	

	
	p_pixel : process(clk, reset)
	begin
		if reset = '1' then
			p_pixel <= VGA_COLOR_BLACK;
		elsif rising_edge(clk) then
			if(end_of_pixel) then
				pixel <= next_pixel;	--mise à jour synchrone pour éviter les délais combinatoires.
			end if;
		end if;
	end process p_pixel;
	

end Behavioral;

