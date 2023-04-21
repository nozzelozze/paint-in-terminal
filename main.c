#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>

#define clear() printf("\033[1;1H\033[2J")
#define default_c "0"
#define HEIGHT 10
#define WIDTH 10

struct Point {
    int color;
};

struct Point canvas[WIDTH][HEIGHT];

struct Command
{
    char evoke[6];
    char info[50];
    void (*command_function)();
};

struct Command new_command(char evoke[6], char info[30], void (*command_function)())
{   
    struct Command command;
    command.command_function = command_function;
    strcpy(command.info, info);
    strcpy(command.evoke, evoke);

    return command;
}

struct Mode 
{
    char name[10];
    int command_count;
    struct Command commands[10];
};

struct Mode new_mode(char name[6], int command_count, ...)
{
    struct Mode mode;
    strcpy(mode.name, name);
    mode.command_count = command_count;
    va_list valist;

    va_start(valist, command_count);

    for (int i = 0; i < command_count; i++)
    {
        mode.commands[i] = va_arg(valist, struct Command);
    }

    va_end(valist);

    return mode;
}

struct Mode *current_mode, main_mode, draw_mode, canvas_mode, file_mode;

void _main() { current_mode = &main_mode; }
void _draw() { current_mode = &draw_mode; }
void _canvas() { current_mode = &canvas_mode; }
void _file() { current_mode = &file_mode; }

int background_color = 15;

char *text(char *foreground, char *background, char *content)
{   
    /*Size of content is max 30.*/

    char *ptr = (char*) malloc(40*sizeof(char));

    sprintf(ptr, "\x1b[38;%s;48;%sm%s\x1b[0m", foreground, background, content);

    return ptr;
}

void pixel(int x, int y, int color)
{
    canvas[x][y].color = color;
}

// --------- Commands -------- //

void draw()
{
    int x, y, color;
    printf("x, y color\n");
    scanf("%d%d%d", &x, &y, &color);
    pixel(x, y, color);
}

void fill()
{
    int x1, y1, x2, y2, color;
    printf("x1, y1, x2, y2 color\n");
    scanf("%d%d%d%d%d", &x1, &y1, &x2, &y2, &color);
    for (int i = x1; i <= x2; i++)
    {
        for (int j = y1; j <= y2; j++)
        {
            pixel(i, j, color);
        }
    }
}

void erase()
{
    int x, y;
    printf("x y\n");
    scanf("%d%d", &x, &y);
    pixel(x, y, background_color);
}

void erasf()
{
    int x1, y1, x2, y2;
    printf("x1, y1, x2, y2\n");
    scanf("%d%d%d%d%d", &x1, &y1, &x2, &y2);
    for (int i = x1; i <= x2; i++)
    {
        for (int j = y1; j <= y2; j++)
        {
            pixel(i, j, background_color);
        }
    }
}

void size()
{
    int width, height;
    printf("w, h\n");
    scanf("%d%d", &width, &height);
    //WIDTH = width;
    //#HEIGHT = height;
}

void bcol()
{   
    int old_bcol;
    old_bcol = background_color;
    printf("color\n");
    scanf("%d", &background_color);
    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {   
            if (canvas[x][y].color == old_bcol)
            {
                canvas[x][y].color = background_color;
            }
        }
    }
}

int calculate_linear_y(k, m, x) {return (int) round((k*x) + m);}
int calculate_linear_x(k, m, y) {return (int) round((y-m)/k);}
void line()
{
    int x1, y1, x2, y2, color;
    printf("x1 y1 x2 y2 color\n");
    scanf("%d%d%d%d%d", &x1, &y1, &x2, &y2, &color);

    int temp_x, temp_y;
    if (x1 > x2)
    {
        temp_x = x2;
        temp_y = y2;
        x2 = x1;
        y2 = y1;
        x1 = temp_x;
        y1 = temp_y;
    }

    float k, m;
    k = (y2 - y1);
    k /= (x2 - x1);
    m = y1 - k*x1;

    // Assuming k != 0
    int x, y, incrmt;
    if (fabs(k) >= 1)
    {
        for (y = y1; y < abs((y2 - y1)+y1+1); /*We increment later*/ )
        {
            pixel(calculate_linear_x(k, m, y), y, color);

            y += 1;
        }
    }

    /*if (y1 < y2)
    {
        incrmt = 1;
    } else
    {
        incrmt = -1;
    }
    for (x = x1; x < abs((x2 - x1)+x1+1); We increment later )
    {   
        int cur_y = (int) round(k*x+m);
        pixel(x, cur_y, color);

        if ((int) round(k*(x+1)+m) > cur_y+incrmt)
        {
            pixel(x, cur_y+incrmt, color);
        }
    }*/
}

void clr()
{
    char answer;
    printf("Are you sure? ( 1 / 0 )\n");
    scanf("%d", &answer);

    if (answer == 1)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            for (int y = 0; y < HEIGHT; y++)
            {
                pixel(x, y, background_color);
            }
        }
    }
}

// ----------------------- //

void init()
{   
    
    clear();
    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            canvas[x][y].color = background_color;
        }
    }

    main_mode = new_mode("Main", 3,  
    new_command("draw", "Open draw mode.", _draw),
    new_command("canvs", "Open canvas mode.", _canvas),
    new_command("file", "Open file mode.", _file)
    );
    draw_mode = new_mode("Draw", 5,  
    new_command("draw", "Draw on the canvas.", draw),
    new_command("fill", "Fill a rectangular area.", fill),
    new_command("erase", "Erase a pixel.", erase),
    new_command("erasf", "Erase an area.", erasf)
    //new_command("line", "Draw a line", line)
    );
    canvas_mode = new_mode("Canvas", 3,
    new_command("bcol", "Change the background color of the canvas.", bcol),
    //new_command("size", "Change the size of the canvas.", size)
    new_command("clr", "Clears the canvas", clr)
    );
    current_mode = &main_mode;

}

void draw_screen()
{
    char *display_title = text(default_c, default_c, (*current_mode).name);
    printf("%s\n", display_title);
    free(display_title);

    char full_canvas[(WIDTH*HEIGHT*18)+HEIGHT*3+WIDTH*3]; // One pixel is max 18 in length. Includes \n and numbers
    memset(full_canvas, 0, sizeof(full_canvas));

    char x_indicator[WIDTH*3];
    memset(x_indicator, 0, sizeof(x_indicator));
    strcat(x_indicator, "  ");
    for (int i = 0; i < WIDTH; i++)
    {   
        char current[5];
        memset(current, 0, sizeof(current));
        sprintf(current, "%d", i);
        if (i < 10)
        {
            strcat(current, " ");
        }
        strcat(x_indicator, current);
    }
    strcat(full_canvas, x_indicator);
    strcat(full_canvas, "\n");

    for (int y = 0; y < HEIGHT; y++)
    {   
        char y_indicator[3];
        memset(y_indicator, 0, sizeof(y_indicator));
        sprintf(y_indicator, "%d ", y);
        strcat(full_canvas, y_indicator);
        for (int x = 0; x < WIDTH; x++)
        {   
            char color[18];
            sprintf(color, "\x1b[48;5;%dm  \x1b[0m", canvas[x][y].color);
            strcat(full_canvas, color);
        }
        strcat(full_canvas, "\n");
    }

    printf("%s\n", full_canvas);

    for (int i = 0; i < (*current_mode).command_count; i++)
    {

        char command_info[50];
        memset(command_info, 0, sizeof(command_info));
        strcat(command_info, (*current_mode).commands[i].evoke);

        printf("%s\n", command_info);
    }
    printf("\n");

}

void take_input()
{
    char command[6];
    scanf("%6s", &command);

    if (strcmp(command, "back") == 0 && (*current_mode).name != "Main")
    {
        _main();
    }

    for (int i = 0; i < 10; i++)
    {
        if (strcmp(command, (*current_mode).commands[i].evoke) == 0)
        {   
            printf("\n");
            (*(*current_mode).commands[i].command_function)();
        }
    }

}

int main()
{
    init();

    while (1) 
    {   
        clear();
        draw_screen();
        take_input();
    }

    return 0;
}