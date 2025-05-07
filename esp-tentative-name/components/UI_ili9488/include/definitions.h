/* LCD size */
#define LCD_H_RES   (480)
#define LCD_V_RES   (320)

/* LCD settings */
//#define LCD_SPI_NUM         (SPI2_HOST)
#define LCD_PIXEL_CLK_HZ    (40 * 1000 * 1000) //display refresh rate
#define LCD_CMD_BITS        (8)
#define LCD_PARAM_BITS      (8)
#define LCD_COLOR_SPACE     (ESP_LCD_COLOR_SPACE_RGB)
#define LCD_BITS_PER_PIXEL  (18)
#define LCD_DRAW_BUFF_DOUBLE (0) //enables double buffering
#define LCD_DRAW_BUFF_HEIGHT (50) //draw buffer height
#define LCD_BL_ON_LEVEL     (1) //backlight on level
                                        
/* LCD pins */
// Sharing SCLK and MOSI with SD card (see database.h)
//#define LCD_GPIO_SCLK       (GPIO_NUM_8)
//#define LCD_GPIO_MOSI       (GPIO_NUM_10)
#define LCD_GPIO_RST        (GPIO_NUM_4)
#define LCD_GPIO_DC         (GPIO_NUM_5)
#define LCD_GPIO_CS         (GPIO_NUM_3)
#define LCD_GPIO_BL         (GPIO_NUM_2)

