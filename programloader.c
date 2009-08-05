#include <stdio.h>
#include <string.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <lua.h>
#include <lauxlib.h>
#include <SDL_ttf.h>

#define MAXMEMBERS 10
#define MAXARGS	10
#define ARGSLIMITS 5



void My_CreateWindow();
void callback_button_exit();
static void My_Action();
void My_ParseLuaConfig(lua_State *L);
static void My_ActionStart(AG_Event *event);


typedef struct program_ {
	char *name;
	int argsnum;
	char *args;
	char *argstype;
	char *dir;
	char *usage;
	char *intro;
	int runtime;
} Program;



char *typeptr[MAXARGS];
char *argsptr[MAXARGS];
char *textbuf[MAXARGS];

AG_Console *console;
char buf[1024]; 
Program ap;

int main()
{
	// declare a lua thread state
	lua_State *L;
	
	// create a new lua state
	L = luaL_newstate();
	// open standard libs
	luaL_openlibs(L);
	
	LUA_LoadFile(L, "config.txt");
	My_ParseLuaConfig(L);
	
	if (AG_InitCore("ProgramLoader", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return 1;		
	}
	
	if (AG_InitVideo(600, 480, 32, AG_VIDEO_RESIZABLE) == -1) { //AG_VIDEO_RESIZABLE AG_VIDEO_FULLSCREEN
		fprintf(stderr, "%s\n", AG_GetError());
		return -1;
	}
	
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_Quit);

	// set font
	AG_SetString(agConfig, "font-path", "/root/.fonts/");
	AG_Font *font = AG_FetchFont("msyh.ttf", 14, 0);
	if (font != NULL) {
		agDefaultFont = font;
		AG_TextFont(font);
	}
	else 
		fprintf(stderr, "%s\n", AG_GetError());
		
	My_CreateWindow();
	
	AG_EventLoop();
	AG_Destroy();
	lua_close(L);
	return 0;
}

void My_ParseLuaConfig( lua_State *L )
{
	void *p;
	char *p1;
	char *str;
	int len, n;
	int i = 0;
	int a1, a2;
	
	str = (char *)LUA_GetTS( L, "Program", "name" );
	len = strlen(str);
	if ((p = (char *)malloc( len + 1 )) == NULL) {
		fprintf(stderr, "Can't malloc enough memory.\n");
		exit(1);	
	}
	memset(p, 0, sizeof(len + 1));
	// global pointer.
	strcpy(p, str);
	ap.name = p;
	
	n = LUA_GetTN( L, "Program", "argsnum");
	ap.argsnum = n;
	if (ap.argsnum >= MAXARGS) {
		fprintf(stderr, "Arguments number exceed the max value: %d.\n", MAXARGS);
		exit(1);
	}
	
	str = (char *)LUA_GetTS( L, "Program", "argstype" );
	len = strlen(str);
	if ((p = (char *)malloc( len + 1 )) == NULL) {
		fprintf(stderr, "Can't malloc enough memory.\n");
		exit(1);	
	}
	memset(p, 0, sizeof(len + 1));
	// global pointer.
	strcpy(p, str);
	ap.argstype = p;
	
	i = 0;
	p1 = strtok((char *)ap.argstype, " ");
	while (p1) {
		i++;
		if (i >= MAXARGS) {
			fprintf(stderr, "Arguments number exceed the max value: %d.\n", MAXARGS);
			exit(1);
		}
		typeptr[i] = p1;
		p1 = strtok(NULL, " ");
	}
	n = i;
	if (ap.argsnum != n) {
		fprintf(stderr, "Parameter 'argsnum' and 'args' don't match.\n");
		exit(1);
	}
	
	for (i = 1; i<=n; i++) {
		if (!(!strcmp(typeptr[i], "number") || !strcmp(typeptr[i], "string") || !strcmp(typeptr[i], "file"))) {
			fprintf(stderr, "Argument type is not right.\n");
			exit(1);
		}
	}
	
	str = (char *)LUA_GetTS( L, "Program", "args" );
	len = strlen(str);
	if ((p = (char *)malloc( len + 1 )) == NULL) {
		fprintf(stderr, "Can't malloc enough memory.\n");
		exit(1);	
	}
	memset(p, 0, sizeof(len + 1));
	// global pointer.
	strcpy(p, str);
	ap.args = p;
	
	i = 0;
	p1 = strtok((char *)ap.args, " ");
	while (p1) {
		i++;
		if (i >= MAXARGS) {
			fprintf(stderr, "Arguments number exceed the max value: %d.\n", MAXARGS);
			exit(1);
		}
		argsptr[i] = p1;
		p1 = strtok(NULL, " ");
	}
	
}


void My_FileWindow()
{
	AG_Window *win;
	AG_FileDlg *fd;
	AG_FileType *ft;
	
//	AG_TextFont(AG_FetchFont(NULL, 18, 0));
	win = AG_WindowNew(AG_WINDOW_PLAIN|AG_WINDOW_KEEPABOVE);
	
	fd = AG_FileDlgNew(win, AG_FILEDLG_EXPAND|AG_FILEDLG_CLOSEWIN);
	AG_FileDlgSetDirectoryMRU(fd, "binfile-dir", ".");
	AG_FileDlgSetFilename(fd, "pq1d20.bin");
	ft = AG_FileDlgAddType(fd, "EC ROM file", "*.bin", My_Action, NULL);
	
	AG_WindowShow(win);
	
}

void My_CreateWindow()
{	
	AG_Window *win;
	AG_Pane *pane;
	AG_VBox *vbox, *vbox2, *vbox01;
	AG_HBox *hbox1;
	AG_Textbox *tb;
	AG_Label *head, *label1;
	int i;
	
	
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	pane = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND|AG_PANE_FORCE_DIV);
	vbox = AG_VBoxNew(pane->div[0], AG_VBOX_EXPAND);
	
	head = AG_LabelNew(vbox, AG_LABEL_FRAME|AG_LABEL_HFILL, "Program Head");
	AG_LabelJustify(head, AG_TEXT_CENTER);
        AG_LabelValign(head, AG_TEXT_MIDDLE);
	AG_LabelSetFgColor(head, AG_MapRGB(agVideoFmt, 250,250,230));

	label1 = AG_LabelNew(vbox, 0, "  ");
	AG_LabelSizeHint(label1, 1, "XXXXXXXXXXXXX");
	AG_LabelNew(vbox, 0, "  ");

        AG_SeparatorNewHoriz(vbox);	

	
	for (i = 1; i <= ap.argsnum; i++) {
		char buf[128];
		textbuf[i] = (char *)malloc(1024);
		sprintf(buf, "%s: ", argsptr[i]);
		tb = AG_TextboxNew(vbox, AG_TEXTBOX_HFILL, buf);
		AG_TextboxSizeHint(tb, "XXXXXXXXXXX");
		AG_TextboxBindUTF8(tb, textbuf[i], sizeof(1024));
//		AG_WidgetFocus(tb);
	}

	hbox1 = AG_HBoxNew(vbox, AG_HBOX_HFILL|AG_HBOX_HOMOGENOUS);
	AG_ButtonNewFn(hbox1, 0, "Choose file", My_FileWindow, NULL);
	AG_ButtonNewFn(hbox1, 0, "Start", My_ActionStart, NULL);
	AG_ButtonNewFn(hbox1, 0, "Exit", callback_button_exit, NULL);

        AG_SeparatorNewHoriz(vbox);	
	
	console = AG_ConsoleNew(vbox, AG_CONSOLE_EXPAND);
	AG_ConsoleAppendLine(console, "Hello, world!");
	if (ap.name) 
		AG_ConsoleAppendLine(console, ap.name);
	
	vbox2 = AG_VBoxNew(pane->div[1], AG_VBOX_HFILL);
	AG_LabelNew(vbox2, 0, "HELP:");
	AG_LabelNew(vbox2, 0, "This is the help to tool XXX.");

	AG_WindowMaximize(win);
	AG_WindowShow(win);
}

void callback_button_exit()
{
	printf("%s\n", "You clicked exit button!");
	AG_Quit();

}

static void My_Action(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	char *file = AG_STRING(1);
	AG_FileType *ft = AG_PTR(2);
	                        
	if (file != NULL) {
		char *p = strrchr(file, '/');
		p++;
		printf("%s\n", p);
//		strcpy(filename, p);
	}

}

static void *thread1_body(void *arg)
{
	char cmd[1024] = {0};
	sprintf(cmd, "./%s %s %s", ap.name, textbuf[1], textbuf[2]);
	FILE *fd = popen(cmd, "r");
	if (fd) {
		fgets(buf, 1024, fd);
//		printf("Enter My_ActionStart.\n");
		AG_ConsoleAppendLine(console, buf);
	}		
	pclose(fd);
}


static void My_ActionStart(AG_Event *event)
{
	AG_Thread thread1;
	
	AG_ThreadCreate(&thread1, thread1_body, NULL);
}




