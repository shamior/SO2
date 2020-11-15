#define CLEAR_SCREEN    "\x1b[2J"
#define CURSOR_UP_LEFT "\x1b[1;1H"

#define CURSOR_DOWN "\x1b[3B"
#define COLOR_RESET     "\x1b[0m" 
#define CR              COLOR_RESET
// stringifying
#define COLOR_GEN(r,g,b) "\x1b[38;2;" #r ";" #g ";" #b "m"
// Nomes das cores oficiais no ambiente X11 (https://www.html.am/html-codes/color/color-code-chart.cfm)
#define CORAL           COLOR_GEN(255, 127, 80) 
#define LIGHT_SEA_GREEN COLOR_GEN(32, 178, 170) 
#define TOMATO          COLOR_GEN(255, 99, 71) 
#define YELLOW_GREEN    COLOR_GEN(154, 205, 50) 
#define INDIAN_RED      COLOR_GEN(205,92,92) 
#define PURPLE          COLOR_GEN(128,0,128)
#define SPRING_GREEN    COLOR_GEN(0,255,127)