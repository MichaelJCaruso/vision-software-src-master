/*******************************************
 **********	timeSeries.c	************
 *******************************************/

#include "Vk.h"

#include "timeSeries.h"

#include "stdcurses.h"
#include "misc.h"
#include "edit.h"
#include "page.h"
#include "form.h"
#include "rsInterface.h"
#include "spsheet.h"
#include "month.d"
#include "keys.h"
#include "buffers.h"


/*************************************************
 **********	Forward Declarations	**********
 *************************************************/

PrivateFnDef void execInsert ();

PrivateFnDef void execMove ();

PrivateFnDef void moveIt ();

PrivateFnDef void execDelete ();

PrivateFnDef void execReplace ();

PrivateFnDef void execOptions ();

PrivateFnDef void execExpr ();

PrivateFnDef void initTimeSeries ();
PrivateFnDef void initTSSprPage();

PrivateFnDef int validInitial ();
PrivateFnDef void doValidInitial ();

PrivateFnDef void initOptions ();

PrivateFnDef void initBuffer ();

PrivateFnDef void itemList ();


/*************************************************
 **********	Constants and Globals	**********
 *************************************************/

#define COLWIDTH 12
#define TITLEROWS 5
#define FIXEDROWS 1
#define FIXEDCOLS 2
#define BUFFERSIZE  2048

PrivateVarDef FORM *InitForm;
PrivateVarDef MENU::Reference FrequencyMenu, DirectionMenu;

#define COMPANY		(InitForm->field[1])
#define STARTDATE	(InitForm->field[3])
#define OBSERVATIONS	(InitForm->field[5])
#define DIRECTION	(InitForm->field[7])
#define INCREMENT	(InitForm->field[9])
#define FREQUENCY	(InitForm->field[11])

PrivateVarDef LINEBUFFER *DisplayBuf;
PrivateVarDef SPRSHEET *Sprsheet = NULL;
PrivateVarDef PAGE *InitialPage;

PrivateVarDef CUR_WINDOW *Win1, *Win2, *Win3, *SprWin, *AppWin, *StatWin;
PrivateVarDef FORM *Form, *Form1;
PrivateVarDef PAGE *ItemsPage, *ReportPage, *ApplicPage, *PPage;
PrivateVarDef int  doValidate = TRUE, doExec = TRUE, didInitialExec = FALSE;
PrivateVarDef MENU::Reference itemsActionMenu, AppActionMenu;

#define ITEM	(Form->field[2])
#define ITEM_INDX 2
#define HEADING	(Form->field[4])

PrivateVarDef FORM_Field formFields[] = {
 1, 31, (CUR_A_BOLD | CUR_A_UNDERLINE), 18, 0, 'a', "TIME SERIES REPORT", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 4, 11, CUR_A_NORMAL, 5, 0, 'a', "Item:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 4, 17, (CUR_A_DIM | CUR_A_REVERSE), 16, 1, 'a', 
 "                ", " Use Arrow Keys To Select Item, or F1 For Menu", MENU::Reference(), NULL, 
 4, 36, CUR_A_NORMAL, 8, 0, 'a', "Heading:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 4, 45, (CUR_A_DIM | CUR_A_REVERSE), 16, 1, 'a', "                ", 
        " Enter Column(Row) Label For Item", MENU::Reference(), NULL, 
 6, 0, CUR_A_NORMAL, 40, 0, 'a', "----------------------------------------", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 6, 40, CUR_A_NORMAL, 40, 0, 'a', "----------------------------------------", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL,
 7, 27, CUR_A_BOLD, 25, 0, 'a', "Current Report Definition",  
        static_cast<char const*>(NULL),  MENU::Reference(), NULL, 
 8, 5, CUR_A_NORMAL, 3, 0, 'a', "Row", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 8, 12, CUR_A_NORMAL, 4, 0, 'a', "Item", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 8, 29, CUR_A_NORMAL, 7, 0, 'a', "Heading", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 -1, 
};


/***************************************************************
 **********	Menu Definitions 		****************
 ***************************************************************/
PrivateVarDef void addField(), deleteField(), insertField(), replaceField(), 
    moveField(), itemsReportOptions(), clearFields(), specialItem(), lineItem(),
    getItemsParameters(), getOuterParameters(), timeSeriesItems(), 
    outerReportOptions(), getItemsParameters();

PrivateVarDef MENU_Choice actionChoices[] = {
 " Add "       , " Add Field To Report Format "	       , 'a', addField     ,ON,
 " Delete "    , " Delete Field From Report Format "   , 'd', deleteField  ,ON,
 " Insert "    , " Insert Field Into Report Format "   , 'i', insertField  ,ON,
 " Move "      , " Move Field Within Report Format "   , 'm', moveField	   ,ON,
 " Replace "   , " Replace Field In Report Format "    , 'r', replaceField ,ON,
 " Options "   , " Select Report Options "	       , 'o', itemsReportOptions,ON,
 " Parameters "   , " Select Report Parameters"	       , 'p', getItemsParameters,ON,
 " Clear "     , " Delete All Fields In Report Format ", 'c', clearFields  ,ON,
 " Expression ", " Define Item Expression "	       , 'e', specialItem  ,ON,
 " Line "      , " Add Single Line, Double Line or Blank Line "
						       , 'l', lineItem     ,ON,
#if 0
 " File "      , " Save/Retrieve Report Format "       , 'f', NULL	   ,ON,
#endif
 static_cast<char const*>(NULL),
};

PrivateVarDef MENU_Choice reportChoices[] = {
 " Parameters ",   " Change Report Parameters",	'p', getOuterParameters, ON, 
 " Items ",    " Change Report Items to Display", 'i', timeSeriesItems, ON, 
 " Options ",    " Change Report Options", 'o', outerReportOptions, ON, 
 static_cast<char const*>(NULL), 
};

PrivateVarDef int	firstTime = TRUE, gotInitialInput = FALSE, didExec = FALSE;
PublicVarDef  int	inTS = FALSE;
PrivateFnDef void	exec();

PublicFnDef void timeSeries() {
    MENU::Reference menu;
    int i, longest, j;

    if( inTS ) {
    	ERR_displayPause("Time Series already running");
    	return;
    }
    inTS = TRUE;
        
    if( firstTime ) {
    	ERR_displayStr(" Getting Items...",FALSE);
    	firstTime = FALSE;    
	SPR_makeSheet(Sprsheet, COLWIDTH, 1, FIXEDCOLS, TITLEROWS, FIXEDROWS);
	SPR_makeDummySheet(Sprsheet);

/**** create form objects ****/
	FORM_makeForm(Form, formFields, i);

	itemsActionMenu.setTo (new MENU (actionChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
	MENU_title(itemsActionMenu) = " Report Writer:";

/**** initialize option forms and display buffer ****/
	initBuffer();

	menu = MENU_getMenu("companyItemCategoryListList");
	if (menu.isntNil ()) {
            MENU_title(menu) = " Item Categories:";
	    for (i = 0; i < MENU_choiceCount(menu); i++)
		MENU_choiceHandler(menu, i) = itemList;
	}
	FORM_fieldMenu(ITEM) = menu;

	initTimeSeries();
	initOptions();
	ERR_clearMsg();
    }
    SprWin = CUR_newwin(CUR_LINES - 2, CUR_COLS, 0, 0);
    if( STD_hasInsertDeleteLine )
	CUR_idlok(SprWin,TRUE);
    initTSSprPage();

    if( !gotInitialInput )
    {
    	getOuterParameters();
	if( !gotInitialInput )
	{
	    CUR_delwin(SprWin);
	    CUR_delwin(AppWin);
	    CUR_delwin(StatWin);
	    AppActionMenu.clear ();
	    PAGE_deletePage(ReportPage, i);
	    PAGE_deletePage(ApplicPage, i);
	    inTS = FALSE;
	    return;
	}
	else
	    KEY_putc('i');
    }
/**** call page handler ****/

    if( !didExec )
    	PAGE_ExitF8PageParent = TRUE;
    SPR_simulateApplic = TRUE;
    PAGE_handler(ReportPage);
    PAGE_ExitF8PageParent = FALSE;
    CUR_delwin(SprWin);
    CUR_delwin(AppWin);
    CUR_delwin(StatWin);
    AppActionMenu.clear ();
    PAGE_deletePage(ReportPage, i);
    PAGE_deletePage(ApplicPage, i);
           
    inTS = FALSE;
    return;
}

PrivateVarDef int	inItemsPage = FALSE;

PrivateFnDef void timeSeriesItems() {
	int	i;
	Win1 = CUR_newwin(9, CUR_COLS, 0, 0);
	Win2 = CUR_newwin((CUR_LINES-9-1-1), CUR_COLS, 9, 0);
	Win3 = CUR_newwin(1,  CUR_COLS, CUR_LINES-2, 0);
	if( STD_hasInsertDeleteLine )
	    CUR_idlok(Win2,TRUE);
	PAGE_createPage(ItemsPage, 3, NULL, itemsActionMenu, NULL, PAGE_menuType, i);
	PAGE_fkey(ItemsPage, 1) = exec;

	PAGE_createElement(ItemsPage, 0, Form, Win1, PAGE_Input, FORM_handler, TRUE);
	PAGE_createElement(ItemsPage, 1, DisplayBuf, Win2, PAGE_Scroll, BUF_handler, TRUE);
	PAGE_createElement(ItemsPage, 2, NULL, Win3, PAGE_Input, NULL, FALSE);

	inItemsPage = TRUE;
	PAGE_handler(ItemsPage);
	inItemsPage = FALSE;

	CUR_delwin (Win1);
	CUR_delwin (Win2);
	CUR_delwin (Win3);
	PAGE_deletePage(ItemsPage, i);
}

/************************************************
 **********	Menu Functions		*********
 ***********************************************/

PrivateVarDef PAGE *MenuPage;

PrivateFnDef void newField(SPR_Field *fptr) {
    
    strcpy(SPR_item(fptr), FORM_fieldValue(ITEM));
    if (isBlank(FORM_fieldValue(HEADING)))
	strcpy(SPR_heading(fptr), FORM_fieldValue(ITEM));
    else
	strcpy(SPR_heading(fptr), FORM_fieldValue(HEADING));

    FORM_fieldValue(ITEM)[0] = '\0';
    CUR_wattron(Win1, FORM_fieldAttr(ITEM));
    CUR_wmove(Win1, FORM_fieldY(ITEM), FORM_fieldX(ITEM));
    CUR_wprintw(Win1,
    "%-*.*s", FORM_fieldLen(ITEM), FORM_fieldLen(ITEM), 
					FORM_fieldValue(ITEM));
    CUR_wattroff(Win1, FORM_fieldAttr(ITEM));
    FORM_fieldValue(HEADING)[0] = '\0';
    CUR_wattron(Win1, FORM_fieldAttr(HEADING));
    CUR_wmove(Win1, FORM_fieldY(HEADING), FORM_fieldX(HEADING));
    CUR_wprintw(Win1,
    "%-*.*s", FORM_fieldLen(HEADING), FORM_fieldLen(HEADING), 
					FORM_fieldValue(HEADING));
    CUR_wattroff(Win1, FORM_fieldAttr(HEADING));
    FORM_currField(Form) = ITEM_INDX;
}

PrivateFnDef void printField(SPR_Field *fptr, int col) {
    char string[100];
    
/**** NOTE: col must printed as it is used in insert, delete, etc. ****/

    sprintf(string, " %6d     %-16.16s %-16.16s", 
					 col, 
					 SPR_item(fptr),
					 SPR_heading(fptr));
					 
    BUF_appendLine(DisplayBuf, string);
}

PrivateFnDef void displayFields()
{
    if( BUF_lastLine(DisplayBuf) != NULL )
	BUF_changeRow(DisplayBuf, BUF_lastLine(DisplayBuf));
    if (SPR_fieldCount(Sprsheet) > CUR_WIN_rows(Win2))
	BUF_screenRow(DisplayBuf) = CUR_WIN_maxy(Win2);
    else
	BUF_screenRow(DisplayBuf) = SPR_fieldCount(Sprsheet) - 1;
    BUF_paintWindow(DisplayBuf, Win2);
}

/************************************************
 **********	clearFields	*****************
 ***********************************************/
PrivateFnDef void clearFields() {
    int i;
    
    if (SPR_fieldCount(Sprsheet) <= 0)
        return;
	
    for (i = 0; i < SPR_fieldCount(Sprsheet); i++)
        free(SPR_field(Sprsheet, i));
    SPR_fieldCount(Sprsheet) = 0;

    BUF_eraseBuffer(DisplayBuf);
    displayFields();
}


/************************************************
 **********	addField	*****************
 ***********************************************/
PrivateFnDef void addField() {
    SPR_Field *fptr;
    char *ptr;

    if (isBlank(FORM_fieldValue(ITEM)))
    {
        ERR_displayPause(" Please Enter An Item To Print");
	return;
    }
    
    SPR_addField(Sprsheet, ptr);

    fptr =  SPR_field(Sprsheet, SPR_fieldCount(Sprsheet) - 1);   

    newField(fptr);

    printField(fptr, SPR_fieldCount(Sprsheet));

    displayFields();
}


/************************************************
 **********	insertField	*****************
 ***********************************************/
PrivateFnDef void insertField() {
    int i;

    if (SPR_fieldCount(Sprsheet) <= 0)    
    {
        ERR_displayPause(" No Fields To Insert Before");
	return;
    }
    
    if (isBlank(FORM_fieldValue(ITEM)))
    {
        ERR_displayPause(" Please Enter An Item To Print");
	return;
    }

    ERR_clearMsg();
    CUR_wmove(Win3, 0, 0);
    CUR_wprintw(Win3, 
    " Use Arrow Keys To Select INSERT Position, Execute(F2), Quit(F9)");

    PAGE_createPage(MenuPage, 3, NULL, NULL, NULL, PAGE_noType, i);
    PAGE_fkey(MenuPage, 1) = execInsert;
    
    PAGE_createElement(MenuPage, 0, NULL, Win1, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 1, DisplayBuf, Win2,
				    PAGE_Scroll, BUF_handler, TRUE);
    PAGE_createElement(MenuPage, 2, NULL, Win3, PAGE_Init, NULL, FALSE);

    PAGE_status(MenuPage) = PAGE_ExitOnExec;
    BUF_status(DisplayBuf) = BUF_Hilight;

    PAGE_handler(MenuPage);

    BUF_status(DisplayBuf) = BUF_Normal;
    CUR_werase(Win3);
    PAGE_deletePage(MenuPage, i);
}

PrivateFnDef void execInsert () {
    char *ptr;
    int i, field;
    SPR_Field *fptr, *holdfptr;

    ptr = BUF_line(BUF_row(DisplayBuf));
    field = atoi(ptr) - 1;

    SPR_addField(Sprsheet, ptr);
    holdfptr = SPR_field(Sprsheet, (SPR_fieldCount(Sprsheet) - 1));
    
    for (i = (SPR_fieldCount(Sprsheet) - 1); i > field; i--)
        SPR_field(Sprsheet, i) = SPR_field(Sprsheet, i - 1);
	

    SPR_field(Sprsheet, field) = holdfptr;
    fptr =  SPR_field(Sprsheet, field);   
    newField(fptr);

    BUF_eraseBuffer(DisplayBuf);
    for (i = 0; i < SPR_fieldCount(Sprsheet); i++)
    {
        fptr = SPR_field(Sprsheet, i);
	printField(fptr, (i + 1));
    }

    displayFields();
}


/************************************************
 **********	moveField	*****************
 ***********************************************/
PrivateFnDef void moveField() {
    int i;

    if (SPR_fieldCount(Sprsheet) <= 0)    
    {
        ERR_displayPause(" No Fields To Move");
	return;
    }
    
    ERR_clearMsg();
    CUR_wmove(Win3, 0, 0);
    CUR_wprintw(Win3, 
	" Use Arrow Keys To Select Field To MOVE, Execute(F2), Quit(F9)");

    PAGE_createPage(MenuPage, 3, NULL, NULL, NULL, PAGE_noType, i);
    PAGE_fkey(MenuPage, 1) = execMove;
    
    PAGE_createElement(MenuPage, 0, NULL, Win1, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 1, DisplayBuf, Win2,
				    PAGE_Scroll, BUF_handler, TRUE);
    PAGE_createElement(MenuPage, 2, NULL, Win3, PAGE_Init, NULL, FALSE);

    PAGE_status(MenuPage) = PAGE_ExitOnExec;
    BUF_status(DisplayBuf) = BUF_Hilight;

    PAGE_handler(MenuPage);

    BUF_status(DisplayBuf) = BUF_Normal;
    CUR_werase(Win3);
    PAGE_deletePage(MenuPage, i);
}

PrivateVarDef int FieldToMove;

PrivateFnDef void execMove () {
    char *ptr;
    
    ptr = BUF_line(BUF_row(DisplayBuf));
    FieldToMove = atoi(ptr) - 1;

    ERR_clearMsg();
    CUR_wmove(Win3, 0, 0);
    CUR_wprintw(Win3, 
	" Use Arrow Keys To Select MOVE Position, Execute(F2), Quit(F9)");
    PAGE_fkey(MenuPage, 1) = moveIt;
    PAGE_handler(MenuPage);
}

PrivateFnDef void moveIt () {
    char *ptr;
    int i, insField;
    SPR_Field *fptr, *holdfptr;

    ptr = BUF_line(BUF_row(DisplayBuf));
    insField = atoi(ptr) - 1;

    holdfptr = SPR_field(Sprsheet, FieldToMove);
    
    if (FieldToMove > insField)
    {
	for (i = FieldToMove; i > insField; i--)
	    SPR_field(Sprsheet, i) = SPR_field(Sprsheet, i - 1);
	SPR_field(Sprsheet, insField) = holdfptr;
    }
    else if (FieldToMove < insField)
    {
	for (i = FieldToMove; i < (insField - 1); i++)
	    SPR_field(Sprsheet, i) = SPR_field(Sprsheet, i + 1);
	SPR_field(Sprsheet, (insField - 1)) = holdfptr;
    }
    else
	return;
	
    BUF_eraseBuffer(DisplayBuf);
    for (i = 0; i < SPR_fieldCount(Sprsheet); i++)
    {
        fptr = SPR_field(Sprsheet, i);
	printField(fptr, (i + 1));
    }
    displayFields();
}



/**************************************************
 **********	deleteField	*******************
 *************************************************/
PrivateFnDef void deleteField() {
    int i;

    if (SPR_fieldCount(Sprsheet) <= 0)    
    {
        ERR_displayPause(" No Fields To Delete");
	return;
    }
    
    ERR_clearMsg();
    CUR_wmove(Win3, 0, 0);
    CUR_wprintw(Win3, 
	" Use Arrow Keys To Select Field To DELETE, Execute(F2), Quit(F9)");

    PAGE_createPage(MenuPage, 3, NULL, NULL, NULL, PAGE_noType, i);
    PAGE_fkey(MenuPage, 1) = execDelete;
    
    PAGE_createElement(MenuPage, 0, NULL, Win1, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 1, DisplayBuf, Win2,
				    PAGE_Scroll, BUF_handler, TRUE);
    PAGE_createElement(MenuPage, 2, NULL, Win3, PAGE_Init, NULL, FALSE);

    PAGE_status(MenuPage) = PAGE_ExitOnExec;
    BUF_status(DisplayBuf) = BUF_Hilight;

    PAGE_handler(MenuPage);

    BUF_status(DisplayBuf) = BUF_Normal;
    CUR_werase(Win3);
    PAGE_deletePage(MenuPage, i);
}

PrivateFnDef void execDelete () {
    char *ptr;
    int i, field;
    SPR_Field *fptr;

    ptr = BUF_line(BUF_row(DisplayBuf));
    field = atoi(ptr) - 1;

    free(SPR_field(Sprsheet, field));
    
    if ((field + 1) != SPR_fieldCount(Sprsheet))
    {
	SPR_fieldCount(Sprsheet)--;
        for (i = field; i < SPR_fieldCount(Sprsheet); i++)
	    SPR_field(Sprsheet, i) = SPR_field(Sprsheet, i + 1);
    }
    else
	SPR_fieldCount(Sprsheet)--;
    

    BUF_eraseBuffer(DisplayBuf);
    for (i = 0; i < SPR_fieldCount(Sprsheet); i++)
    {
        fptr = SPR_field(Sprsheet, i);
	printField(fptr, (i + 1));
    }

    displayFields();
}


/************************************************
 **********	replaceField	*****************
 ***********************************************/
PrivateFnDef void replaceField() {
    int i;

    if (SPR_fieldCount(Sprsheet) <= 0)    
    {
        ERR_displayPause(" No Fields To Replace");
	return;
    }
    
    if (isBlank(FORM_fieldValue(ITEM)))
    {
        ERR_displayPause(" Please Enter An Item To Print");
	return;
    }
    
    ERR_clearMsg();
    CUR_wmove(Win3, 0, 0);
    CUR_wprintw(Win3, 
	" Use Arrow Keys To Select Field To REPLACE, Execute(F2), Quit(F9)");

    PAGE_createPage(MenuPage, 3, NULL, NULL, NULL, PAGE_noType, i);
    PAGE_fkey(MenuPage, 1) = execReplace;
    
    PAGE_createElement(MenuPage, 0, NULL, Win1, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 1, DisplayBuf, Win2,
				    PAGE_Scroll, BUF_handler, TRUE);
    PAGE_createElement(MenuPage, 2, NULL, Win3, PAGE_Init, NULL, FALSE);

    PAGE_status(MenuPage) = PAGE_ExitOnExec;
    BUF_status(DisplayBuf) = BUF_Hilight;

    PAGE_handler(MenuPage);

    BUF_status(DisplayBuf) = BUF_Normal;
    CUR_werase(Win3);
    PAGE_deletePage(MenuPage, i);
}

PrivateFnDef void execReplace () {
    char *ptr;
    int i, field;
    SPR_Field *fptr;

    ptr = BUF_line(BUF_row(DisplayBuf));
    field = atoi(ptr) - 1;
    fptr =  SPR_field(Sprsheet, field);   
    newField(fptr);

    BUF_eraseBuffer(DisplayBuf);
    for (i = 0; i < SPR_fieldCount(Sprsheet); i++)
    {
        fptr = SPR_field(Sprsheet, i);
	printField(fptr, (i + 1));
    }
    displayFields();
}



/****************************************************
 **********	reportOptions		*************
 ***************************************************/
#define TITLE1		(Form1->field[1])
#define TITLE2		(Form1->field[2])

#define ANALYSIS	(Form1->field[4])
#define RELATIVE	(Form1->field[6])
#define FORMAT		(Form1->field[8])


PrivateVarDef FORM_Field form1Fields[] = {
 1, 4, CUR_A_NORMAL, 6, 0, 'a', "Title:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 1, 11, (CUR_A_DIM | CUR_A_REVERSE), 43, (FORM_InputFlag|FORM_ScrollFlag), 'a', 
"                                        ", " Enter Report Title", MENU::Reference(), NULL, 
 2, 11, (CUR_A_DIM | CUR_A_REVERSE), 43, (FORM_InputFlag|FORM_ScrollFlag), 'a', 
"                                        ", " Enter Report Title", MENU::Reference(), NULL, 
 4, 1, CUR_A_NORMAL, 9, 0, 'a', "Analysis:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 4, 11, (CUR_A_DIM | CUR_A_REVERSE), 12, 1, 'm', "", 
        " Use Arrow Keys To Analysis Type, or F1 For Menu", MENU::Reference(), NULL, 
 4, 25, CUR_A_NORMAL, 12, 0, 'a', "Relative To:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 4, 38, (CUR_A_DIM | CUR_A_REVERSE), 16, 1, 'a', "", 
        " Enter Item For Analysis, or Press F1 For Menu", MENU::Reference(), NULL, 
 6, 3, CUR_A_NORMAL, 7, 0, 'a', "Format:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 6, 11, (CUR_A_DIM | CUR_A_REVERSE), 10, 1, 'm', "", 
        " Use Arrow Keys To Select Report Format, or F1 For Menu", MENU::Reference(), NULL, 
-1, 
};

PrivateVarDef MENU_Choice analysisChoices[] = {
 " Standard ",	 " Standard Analysis",          's', FORM_menuToForm, ON,
 " Percent Of ", " Percent Of Analysis",	'p', FORM_menuToForm, ON,
 " Trend ",	 " Trend Analysis",		't', FORM_menuToForm, ON,
 static_cast<char const*>(NULL),
};

PrivateVarDef MENU_Choice directionChoices[] = {
 " Reverse",	" Reverse Order From Start Date",   'r', FORM_menuToForm, ON, 
 " Forward",	" Forward Order From Start Date",   'f', FORM_menuToForm, ON, 
 static_cast<char const*>(NULL), 
};

PrivateVarDef MENU_Choice freqChoices[] = {
 " years "		, " Annual Frequency"	, '\0', FORM_menuToForm, ON,
 " yearEnds "		, " Annual Frequency"	, '\0', FORM_menuToForm, ON,
 " yearBeginnings "	, " Annual Frequency"	, '\0', FORM_menuToForm, ON,
 " quarters "		, " Quarterly Frequency", '\0', FORM_menuToForm, ON,
 " quarterEnds "	, " Quarterly Frequency", '\0', FORM_menuToForm, ON,
 " quarterBeginnings "	, " Quarterly Frequency", '\0', FORM_menuToForm, ON,
 " months "		, " Monthly Frequency"	, '\0', FORM_menuToForm, ON,
 " monthEnds "		, " Monthly Frequency"	, '\0', FORM_menuToForm, ON,
 " monthBeginnings "	, " Monthly Frequency"	, '\0', FORM_menuToForm, ON,
 " days "		, " Daily Frequency"	, '\0', FORM_menuToForm, ON,
 " businessDays "	, " Daily Frequency"	, '\0', FORM_menuToForm, ON,
 static_cast<char const*>(NULL), 
};

PrivateVarDef MENU_Choice formChoices[] = {
 " Standard ",  " Print Items As Rows, Years As Columns", 's',
      FORM_menuToForm, ON, 
 " Flipped ", " Print Items As Columns, Years As Rows", 'f',
      FORM_menuToForm, ON, 
 static_cast<char const*>(NULL), 
};


PrivateFnDef int reportOptions(int useOuterPage) {
    CUR_WINDOW *form1Win, *form2Win;
    int i;

/*** create form windows ****/
    form1Win = CUR_newwin(10, 57, CUR_LINES-10-3, 2);
    WIN_ReverseBox(form1Win,NULL);
    CUR_wmove(form1Win,0,19);
    CUR_wattron(form1Win,CUR_A_REVERSE);
    CUR_wprintw(form1Win,"TIME SERIES OPTIONS");
    CUR_wmove(form1Win,CUR_WIN_maxy(form1Win),2);
    CUR_wprintw(form1Win,"Save Options(F2)  Quit(F9)");
    CUR_wattroff(form1Win,CUR_A_REVERSE);
    form2Win = CUR_subwin(form1Win, 8, 55, CUR_LINES-10-2, 3);

/**** create page object ****/
    if( useOuterPage )
    {
	PAGE_createPage(MenuPage, 5, NULL, NULL, NULL, PAGE_noType, i);    
	PAGE_createElement(MenuPage, 0, NULL, SprWin, PAGE_Init, NULL, FALSE);
	PAGE_createElement(MenuPage, 1, NULL, AppWin, PAGE_Init, NULL, FALSE);
	PAGE_createElement(MenuPage, 4, NULL, StatWin, PAGE_Init, NULL, FALSE);
	i = 2;
    }
    else
    {
	PAGE_createPage(MenuPage, 5, NULL, NULL, NULL, PAGE_noType, i);
	PAGE_createElement(MenuPage, 0, NULL, Win1, PAGE_Init, NULL, FALSE);
	PAGE_createElement(MenuPage, 1, NULL, Win2, PAGE_Init, NULL, FALSE);
	PAGE_createElement(MenuPage, 2, NULL, Win3, PAGE_Init, NULL, FALSE);
	i = 3;
    }
    PAGE_fkey(MenuPage, 1) = execOptions;
    PAGE_createElement(MenuPage, i, NULL, form1Win, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, (i+1), Form1, form2Win,
					 PAGE_Input, FORM_handler, TRUE);

/**** call page handler ****/

    PAGE_handler(MenuPage);
           
/*****  cleanup  *****/

    CUR_delwin (form2Win);
    CUR_delwin (form1Win);

    PAGE_deletePage (MenuPage, i);

    return (FALSE);
}

PrivateVarDef int	optionsExec = FALSE;

PrivateFnDef void outerReportOptions() {
	optionsExec = TRUE;
	reportOptions(TRUE);
	optionsExec = FALSE;
}

PrivateFnDef void itemsReportOptions() {
	reportOptions(FALSE);
}

PrivateFnDef void execOptions () {
    if( optionsExec )
    {
	FORM_fieldValue(ITEM)[0] = '\0';
    	exec();
    }
    PAGE_status(MenuPage) = PAGE_ExitOnExec;
}


/************************************************
 *********	specialItem	*****************
 ************************************************/
#define EXPR1	    (ExprForm->field[1])
#define EXPR2	    (ExprForm->field[2])

PrivateVarDef FORM_Field exprFields[] = {
 2, 1, CUR_A_NORMAL, 11, 0, 'a', "Expression:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 2, 13, (CUR_A_DIM | CUR_A_REVERSE), 40, 1, 'a',
 "                                        ", " Enter Expression", MENU::Reference(), NULL, 
 3, 13, (CUR_A_DIM | CUR_A_REVERSE), 40, 1, 'a',
 "                                        ", " Enter Expression", MENU::Reference(), NULL, 
 7, 5, CUR_A_NORMAL, 29, 0, 'a', "Execute(F2)  Quit(F9)" , 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
-1, 
};

PrivateVarDef FORM *ExprForm;

PrivateFnDef void specialItem() {
    int i;
    CUR_WINDOW *win, *swin;
    CUR_WINDOW *MenuWin;
    
    win = CUR_newwin(10, 60, 10, 0);
    WIN_ReverseBox(win,NULL);
    swin = CUR_subwin(win, 8, 58, 11, 1);
    MenuWin = CUR_newwin(11, 21, 5, 55);

    FORM_makeForm(ExprForm, exprFields, i);
    FORM_fieldValue(EXPR1)[0] = '\0';
    FORM_fieldValue(EXPR2)[0] = '\0';
    
    PAGE_createPage(MenuPage, 5, NULL, NULL, NULL, PAGE_noType, i);
    PAGE_fkey(MenuPage, 1) = execExpr;

    PAGE_createElement(MenuPage, 0, NULL, Win1, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 1, NULL, Win2, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 2, NULL, MenuWin,
				 PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 3, NULL, win, PAGE_Init, NULL, FALSE);
    PAGE_createElement(MenuPage, 4, ExprForm, swin,
				 PAGE_Input, FORM_handler, TRUE);
    PAGE_handler(MenuPage);

    PAGE_deletePage(MenuPage, i);
    CUR_delwin(MenuWin);
    CUR_delwin(swin);
    CUR_delwin(win);
    free(ExprForm);
}

PrivateFnDef void execExpr () {
    char buffer[256];

    if (isBlank(FORM_fieldValue(EXPR1)) &&
	isBlank(FORM_fieldValue(EXPR2)))
    {
        ERR_displayPause (" Please Enter An Expression");
	return;
    }
    
    PAGE_status(MenuPage) = PAGE_ExitOnExec;

    snprintf(buffer, sizeof(buffer), "%s %s", FORM_fieldValue(EXPR1), FORM_fieldValue(EXPR2));
    strcpy(FORM_fieldValue(ITEM), buffer);
    CUR_wattron(Win1, FORM_fieldAttr(ITEM));
    CUR_wmove(Win1, FORM_fieldY(ITEM), FORM_fieldX(ITEM));
    CUR_wprintw(Win1,
    "%-*.*s", FORM_fieldLen(ITEM), FORM_fieldLen(ITEM), 
					FORM_fieldValue(ITEM));
    CUR_wattroff(Win1, FORM_fieldAttr(ITEM));
}


/***************************************************
 **********	lineItem		************
 ***************************************************/
PrivateVarDef void execLine();

PrivateVarDef MENU_Choice lineChoices[] = {
 " Single ",    " Add Single Line To Report Format ", 's', execLine, ON, 
 " Double ",    " Add Double Line To Report Format",  'd', execLine, ON, 
 " Blank ",     " Add Blank Line To Report Format",   'b', execLine, ON, 
 static_cast<char const*>(NULL), 
};

PrivateVarDef MENU::Reference LineMenu;

PrivateFnDef void lineItem() {
    int i, j, longest;
    CUR_WINDOW *SysWin;
    
    SysWin = CUR_newwin(10, 20, 5, 5);
    
    LineMenu.setTo (new MENU (lineChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
    MENU_title(LineMenu) = " Line Types:";
    MENU_handler(LineMenu, SysWin, PAGE_Input);
    CUR_delwin(SysWin);
    LineMenu.clear ();
}

#define fillField(fld, c) {\
    for (i = 0; i < COLWIDTH; i++)\
	fld[i] = c;\
    fld[i] = '\0';\
}

PrivateFnDef void execLine() {
    int i;
    char *ptr;
    SPR_Field *fptr;

    SPR_addField(Sprsheet, ptr);
    fptr =  SPR_field(Sprsheet, SPR_fieldCount(Sprsheet) - 1);   

    switch (MENU_choiceLetter(LineMenu, MENU_currChoice(LineMenu)))
    {
    case 's':
	fillField(SPR_item(fptr), '-');
        strcpy(SPR_heading(fptr), "(single line)");
        break;

    case 'd':
	fillField(SPR_item(fptr), '=');
        strcpy(SPR_heading(fptr), "(double line)");
        break;

    case 'b':
	fillField(SPR_item(fptr), ' ');
        strcpy(SPR_heading(fptr), "(blank line)");
        break;

    default:
	return;
    }
    
    SPR_fieldType(fptr) = SPR_Line;
    
    printField(fptr, SPR_fieldCount(Sprsheet));

    displayFields();
}



/***************************************************
 **********	Exec Functions		************
 ***************************************************/
#define centerTitle(str, t)\
{\
    n = strlen(t);\
    width = ((80 - n) / 2) + n;\
    sprintf(str, "%*s%*s", width, t, COLWIDTH, " ");\
}

#define determineType()\
{\
    pctOf = 'P' == FORM_fieldValue(ANALYSIS)[1];\
    trend = 'T' == FORM_fieldValue(ANALYSIS)[1];\
}


PrivateFnDef int createReport()
{
    SPR_Field *field;
    char string[RS_MaxLine], title[200];
    int n, i, observations, increment, width, flipped, pctOf, trend;

    determineType ();
    if (pctOf && isBlank (FORM_fieldValue (RELATIVE)))
    {
	ERR_displayPause(" Percent-of report requires 'Relative' value");
	return(TRUE);
    }

    RS_sendOnly("FrontEndTools StandardReport named __tmpReport send: [");
    flipped = FALSE;
    if (0 == strcmp(" Standard ", FORM_fieldValue(FORMAT)))
    {
	sprintf(string, ":transpose <- ^global TRUE;");
	flipped = TRUE;
    }
    else
	sprintf(string, ":transpose <- ^global FALSE;");
    RS_sendOnly(string);

    centerTitle(title, FORM_fieldValue(TITLE1));
    sprintf(string, ":title <- \"%s", title);
    RS_sendOnly(string);
    centerTitle(title, FORM_fieldValue(TITLE2));
    sprintf(string, "%s\";", title);
    RS_sendOnly(string);

/***** assign items *****/
    RS_sendOnly(":itemspecList <- ");
    for (i = 0; i < SPR_fieldCount(Sprsheet); i++)
    {
	field = SPR_field(Sprsheet, i);
        RS_sendOnly("( ^global FrontEndTools ItemSpec new do: [");
	if (SPR_fieldType(field) == SPR_Line)
	{
	    sprintf(string, ":block <- [ \" %s \" ];", SPR_item(field));
	    RS_sendOnly(string);
	    if (flipped)
		sprintf(string, ":label <- \"%*.*s\";", 
		    (COLWIDTH * 2), (COLWIDTH * 2), " ");
	    else
		sprintf(string, ":label <- \"%*.*s\";", 
		    COLWIDTH, COLWIDTH, " ");
	    RS_sendOnly(string);
	}
	else
	{
	    if (pctOf)
		sprintf(string, ":block <- [ (%s) / (%s) * 100 ];", 
		    SPR_item(field), FORM_fieldValue(RELATIVE));
	    else if (trend)
		sprintf(string, ":block <- [ [%s] pctChangeLag: %s %s ];", 
		    SPR_item(field),
		    FORM_fieldValue (INCREMENT),
		    FORM_fieldValue (FREQUENCY) );
	    else
		sprintf(string, ":block <- [ %s ];", SPR_item(field));
	    RS_sendOnly(string);
	    if (flipped)
		sprintf(string, ":label <- \"%-*.*s\";", 
		    (COLWIDTH * 2), (COLWIDTH * 2), SPR_heading(field));
	    else
		sprintf(string, ":label <- \"%*.*s\";", 
		    COLWIDTH, COLWIDTH, SPR_heading(field));
	    RS_sendOnly(string);
	}
	if (i == (SPR_fieldCount(Sprsheet) - 1))
	    RS_sendOnly("] ); ");
	else
	    RS_sendOnly("] ), ");
    }
    
    if (TRUE == RS_sendAndCheck("];", ">>>"))
    {
	ERR_displayPause(" Error assigning report format");
	return(TRUE);
    }

/**** run report ****/
    observations = atoi (FORM_fieldValue (OBSERVATIONS));
    increment	 = atoi (FORM_fieldValue (INCREMENT));

    sprintf
	(string,
	 "FrontEndTools StandardReport named __tmpReport\
	    forEntity: Named Company %s\
	    withDateRange: (%s to: %s asDate %c %d %s by: %s %s);",
	 FORM_fieldValue (COMPANY),
	 FORM_fieldValue (STARTDATE),
	 FORM_fieldValue (STARTDATE),
	 FORM_fieldValue (DIRECTION)[1] == 'R' ? '-' : '+',
	 (observations - 1) * increment,
	 FORM_fieldValue (FREQUENCY),
	 FORM_fieldValue (INCREMENT),
	 FORM_fieldValue (FREQUENCY));

    RS_sendLine(string);

    if (flipped)
    {
	SPR_fixedCols(Sprsheet) = 2;
	SPR_colCount(Sprsheet) =
        ((observations + 2) > ((CUR_COLS / COLWIDTH) + 1))
	    ? (observations + 2) : ((CUR_COLS / COLWIDTH) + 1);
    }
    else
    {
	SPR_fixedCols(Sprsheet) = 1;
	SPR_colCount(Sprsheet) = 
        ((SPR_fieldCount(Sprsheet) + 1) > ((CUR_COLS / COLWIDTH) + 1))
	    ? (SPR_fieldCount(Sprsheet) + 1) : ((CUR_COLS / COLWIDTH) + 1);
    }

    SPR_erase(Sprsheet);
    if (SPR_readSSheet(Sprsheet))
    {
	 ERR_displayPause(" Error reading spread sheet");
	 return TRUE;
    }

    return FALSE;
}

#if 0
PrivateVarDef int printReport();

PrivateVarDef MENU_Choice reportChoices[] = {
 " Print ",   " Print Hardcopy Of Report",	'p', printReport, ON, 
 " Save ",    " Save Report Format",		's', NULL, ON, 
 " Download ", " Download Report File To PC",	'd', NULL, ON, 
 static_cast<char const*>(NULL), 
};
#endif

PrivateFnDef void reportFileMenu()
{
	EDIT_reportFileMenu(ReportPage,FALSE);
}

PrivateFnDef void applicFileMenu()
{
	EDIT_reportFileMenu(ApplicPage,FALSE);
}

PrivateFnDef void exec() {
    int i, j, longest;
    CUR_WINDOW *win;
    MENU::Reference actionMenu;

    if (! isBlank (FORM_fieldValue (ITEM)))
    {
        addField ();
	return;
    }

    if (SPR_fieldCount(Sprsheet) <= 0)
    {
        ERR_displayPause(" Please Enter Items To Print");
	return;
    }

    if (doValidate)
    {
    	doExec = FALSE;
	if(!validInitial())
    	    return;
    	doExec = TRUE;
    }

/**** create the report ****/
    ERR_displayStr(" Writing report, please wait...",FALSE);

    if (createReport())
        return;
    ERR_clearMsg();
    didExec = TRUE;
    PAGE_ExitF8PageParent = FALSE;

    SPR_handler(Sprsheet, SprWin, PAGE_Init);
    MENU_status(AppActionMenu) = MENU_ExitOnExec;
    if( inItemsPage )
	PAGE_status(ItemsPage) = PAGE_ExitOnExec;
#if 0
    actionMenu.setTo (new MENU (reportChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
    MENU_title(actionMenu) = " Report Writer:";

/**** create page object ****/
    PAGE_createPage(ReportPage, 1, NULL, actionMenu, NULL, PAGE_menuType, i);
    PAGE_fkey(ReportPage, 1) = NULL;
    PAGE_fkey(ReportPage, 4) = reportFileMenu;

/*** create report window ***/
    win = CUR_newwin(CUR_LINES - 1, CUR_COLS, 0, 0);
    if( STD_hasInsertDeleteLine )
	CUR_idlok(win,TRUE);
    PAGE_createElement(ReportPage, 0, Sprsheet, win, PAGE_Scroll, 
					    SPR_handler, TRUE);
/**** call page handler ****/
    PAGE_handler(ReportPage);

/**** cleanup ****/
    SPR_erase(Sprsheet);
    CUR_delwin(win);
    PAGE_deletePage(ReportPage, i);
#endif
}

PrivateFnDef void initTSSprPage() {
    int i, j, longest;

	AppActionMenu.setTo (new MENU (reportChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
	MENU_title(AppActionMenu) = " Time Series:";
	longest += 4;
	if( (i = strlen(MENU_title(AppActionMenu))) >= longest )
	    longest = i + 1;
	i = MENU_choiceCount(AppActionMenu) + 4;
	if( (longest + 55 + 4) > CUR_COLS )
	    j = CUR_COLS - longest - 4;
	else
	    j = 55;
    AppWin = CUR_newwin(i, longest, 5, j);

    PAGE_createPage(ApplicPage, 3, NULL, NULL, NULL, PAGE_noType, i);
    PAGE_createElement(ApplicPage, 0, NULL, SprWin, PAGE_Init, NULL, FALSE)
    PAGE_createElement(ApplicPage, 1, AppActionMenu, AppWin, PAGE_Input, 
					    MENU_handler, TRUE);

/**** create page object ****/
    PAGE_createPage(ReportPage, 2, NULL, ApplicPage, NULL, PAGE_pageType, i);
    PAGE_fkey(ReportPage, 1) = NULL;
    PAGE_fkey(ReportPage, 4) = reportFileMenu;
    PAGE_fkey(ApplicPage, 1) = NULL;
    PAGE_fkey(ApplicPage, 4) = applicFileMenu;

    PAGE_createElement(ReportPage, 0, Sprsheet, SprWin, PAGE_Scroll, 
					    SPR_handler, TRUE);
    StatWin = CUR_newwin(1, CUR_COLS-1, CUR_LINES-2, 0);
    CUR_wattron(StatWin,CUR_A_REVERSE);
    CUR_wmove(StatWin,0,0);
    CUR_wprintw(StatWin,"%-*.*s", CUR_WIN_cols(StatWin), CUR_WIN_cols(StatWin),
    	"  Interface Menu (F5)  Application Menu (F8)  Quit (F9)");
    CUR_wattroff(StatWin,CUR_A_REVERSE);
    PAGE_createElement(ReportPage, 1, NULL, StatWin, PAGE_Init, NULL, FALSE);
    PAGE_createElement(ApplicPage, 2, NULL, StatWin, PAGE_Init, NULL, FALSE);
}


/******************************************************
 ******************	timeSeries	***************
 *****************************************************/
PrivateVarDef FORM_Field initFields[] = {
 1, 4, CUR_A_NORMAL, 8, 0, 'a', "Company:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 1, 13, (CUR_A_DIM | CUR_A_REVERSE), 10, 1, 'a', "", 
	" Enter Company Ticker Symbol ", MENU::Reference(), NULL, 
 3, 1, CUR_A_NORMAL, 11, 0, 'a', "Start Date:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 3, 13, (CUR_A_DIM | CUR_A_REVERSE), 9, 1, 'n', "", 
        " Enter Starting Date", MENU::Reference(), NULL, 
 3, 23, CUR_A_NORMAL, 13, 0, 'a', "Observations:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 3, 37, (CUR_A_DIM | CUR_A_REVERSE), 8, 1, 'n', "5",
        " Enter Number of Observations", MENU::Reference(), NULL, 
 5, 2, CUR_A_NORMAL, 10, 0, 'a', "Direction:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 5, 13, (CUR_A_DIM | CUR_A_REVERSE), 9, 1, 'm', "", 
	" Enter Direction, or Press F1 For Menu", MENU::Reference(), NULL, 
 5, 26, CUR_A_NORMAL, 10, 0, 'a', "Increment:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 5, 37, (CUR_A_DIM | CUR_A_REVERSE), 5, 1, 'n', "1",
	" Enter Increment Value", MENU::Reference(), NULL, 
 7, 2, CUR_A_NORMAL, 10, 0, 'a', "Frequency:",
	static_cast<char const*>(NULL), MENU::Reference(), NULL, 
 7, 13, (CUR_A_DIM | CUR_A_REVERSE), 19, 1, 'm', "", 
	" Enter Frequency, or Press F1 For Menu", MENU::Reference(), NULL, 
-1, 
};

PrivateFnDef void initTimeSeries () {
	int	i, j, longest;
    
	FORM_makeForm(InitForm, initFields, i);
	sprintf(FORM_fieldValue(STARTDATE), " %d", (YEAR - 1901));

	FrequencyMenu.setTo (new MENU (freqChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
	MENU_title(FrequencyMenu) = " Frequency: ";
	FORM_fieldMenu(FREQUENCY) = FrequencyMenu;

	DirectionMenu.setTo (new MENU (directionChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
	MENU_title(DirectionMenu) = " Direction: ";
	FORM_fieldMenu(DIRECTION) = DirectionMenu;
}

PrivateVarDef int	inParamPage = FALSE;

PrivateFnDef int getParameters(int useOuterPage) {
    int i;

    CUR_WINDOW *win, *win2;
    win = CUR_newwin(11, 48, CUR_LINES-11-3, 2);
    WIN_ReverseBox(win,NULL);
    CUR_wmove(win,0,13);
    CUR_wattron(win,CUR_A_REVERSE);
    CUR_wprintw(win,"TIME SERIES PARAMETERS");
    CUR_wmove(win,CUR_WIN_maxy(win),2);
    CUR_wprintw(win,"Execute(F2)  Quit(F9)");
    CUR_wattroff(win,CUR_A_REVERSE);
    win2 = CUR_subwin(win, 9, 46, CUR_LINES-11-2, 3);

    if( useOuterPage )
    {
	PAGE_createPage(PPage, 5, NULL, NULL, NULL, PAGE_noType, i);
	PAGE_createElement(PPage, 0, NULL, SprWin, PAGE_Init, NULL, FALSE);
	PAGE_createElement(PPage, 1, NULL, AppWin, PAGE_Init, NULL, FALSE);
	PAGE_createElement(PPage, 4, NULL, StatWin, PAGE_Init, NULL, FALSE);
	i = 2;
    }
    else
    {
	PAGE_createPage(PPage, 5, NULL, NULL, NULL, PAGE_noType, i);
	PAGE_createElement(PPage, 0, NULL, Win1, PAGE_Init, NULL, FALSE);
	PAGE_createElement(PPage, 1, NULL, Win2, PAGE_Init, NULL, FALSE);
	PAGE_createElement(PPage, 2, NULL, Win3, PAGE_Init, NULL, FALSE);
	i = 3;
    }
    PAGE_createElement(PPage, i, NULL, win, PAGE_Init, NULL, FALSE);
    PAGE_createElement(PPage, (i+1), InitForm, win2,
				 PAGE_Input, FORM_handler, TRUE);
    
    PAGE_fkey(PPage, 1) = doValidInitial;

    inParamPage = TRUE;
    PAGE_handler(PPage);
    inParamPage = FALSE;

    CUR_delwin(win2);
    CUR_delwin(win);
    PAGE_deletePage(PPage, i);
    return(FALSE);
}

PrivateVarDef int	ParamDoExec = TRUE;

PrivateFnDef void getOuterParameters() {
	getParameters(TRUE);
}

PrivateFnDef void getItemsParameters() {
	ParamDoExec = FALSE;
	getParameters(FALSE);
	ParamDoExec = TRUE;
}

PrivateVarDef int UnivFirstTime = TRUE;

PrivateFnDef void doValidInitial () {
    validInitial ();
}

PrivateFnDef int validInitial () {
    char buffer[128];

    ERR_displayStr(" Validating Parameters...",FALSE);
    
    if (isBlank(FORM_fieldValue(COMPANY)))
    {
        ERR_displayPause(" Please Enter Company Ticker Symbol");
	return(FALSE);
    }

    if (isBlank(FORM_fieldValue(STARTDATE)))
    {
        ERR_displayPause(" Please Enter A Start Date");
	return(FALSE);
    }

    if (isBlank(FORM_fieldValue(OBSERVATIONS)))
    {
        ERR_displayPause(" Please Enter A Number Of Observations");
	return(FALSE);
    }

    if (isBlank(FORM_fieldValue(INCREMENT)))
    {
        ERR_displayPause(" Please Enter A Date Increment");
	return(FALSE);
    }

    strToUpper(FORM_fieldValue(COMPANY));
    snprintf(buffer, sizeof(buffer), "Named Company %s", FORM_fieldValue(COMPANY));
    if (RS_sendAndCheck(buffer, ">>>"))
    {
        ERR_displayPause(" Invalid Company");
	return(FALSE);
    }
    
    gotInitialInput = TRUE;
    if( UnivFirstTime )
    	UnivFirstTime = FALSE;
    else
    {
    	doValidate = FALSE;
    	if( doExec && ParamDoExec )
    	    exec();
    	doValidate = TRUE;
    }

    if( inParamPage )
	PAGE_status(PPage) = PAGE_ExitOnExec;
    return(TRUE);
}




/************************************************
 **********	Misc. Functions		*********
 ***********************************************/

PrivateFnDef void initOptions () {
    int i, longest, j;
    
/**** create form objects ****/
    FORM_makeForm(Form1, form1Fields, i);

    FORM_fieldMenu(RELATIVE) = FORM_fieldMenu(ITEM);
	
    MENU::Reference menu (new MENU (analysisChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
    MENU_title(menu) = " Analysis: ";
    FORM_fieldMenu(ANALYSIS).setTo (menu);

    menu.setTo (new MENU (formChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
    MENU_title(menu) = " Format: ";
    FORM_fieldMenu(FORMAT).setTo (menu);

    FORM_fieldMenu(FREQUENCY) = FrequencyMenu;
    FORM_fieldMenu(DIRECTION) = DirectionMenu;

    strcpy(FORM_fieldValue(FORMAT)	, formChoices[0].label);
    strcpy(FORM_fieldValue(ANALYSIS)	, analysisChoices[0].label);
}

#if 0
PrivateFnDef validOptions()
{
    char buffer[80];

 #if 0    
    ERR_displayStr(" Validating Options...",FALSE);
    
    if (isBlank(FORM_fieldValue(COMPANY)))
    {
        ERR_displayPause(" Please Enter Company Ticker Symbol");
	return(FALSE);
    }

    strToUpper(FORM_fieldValue(COMPANY));
    sprintf(buffer, "Named Company %s", FORM_fieldValue(COMPANY));
    if (RS_sendAndCheck(buffer, ">>>"))
    {
        ERR_displayPause(" Invalid Company");
	return(FALSE);
    }
 #endif
    
    return(TRUE);
}
#endif

#if 0
PrivateFnDef int printReport()
{
    SPR_print(Sprsheet, ReportPage);
}
#endif

PrivateFnDef void initBuffer () {
    DisplayBuf = (LINEBUFFER *) malloc(sizeof(LINEBUFFER));
    BUF_maxLineSize(DisplayBuf) = CUR_COLS;
    BUF_initBuffer(DisplayBuf, (10 * BUFFERSIZE));    
}

PrivateFnDef void itemList () {
    int choice, i;
    char string[80];
    CUR_WINDOW *SysWin;
    
    MENU *menu1 = FORM_fieldMenu(ITEM);
    choice = MENU_currChoice(menu1);
    
    sprintf(string, "companyItemCategory%sList", MENU_choiceLabel(menu1, choice).content ());

    MENU *menu2 = MENU_getMenu(string);
    if (menu2 == NULL) {
        ERR_displayPause(" No items for category selected");
	return;
    }
    
    for (i = 0; i < MENU_choiceCount(menu2); i++)
	MENU_choiceHandler(menu2, i) = FORM_menuToForm;
    MENU_title(menu2) = " Items: ";

    SysWin = CUR_newwin(10, 20, 5, 5);
    
    FORM_fieldMenu(ITEM) = menu2;
    FORM_fieldMenu(RELATIVE).setTo (menu2);
    MENU_handler(menu2, SysWin, PAGE_Input);
    FORM_fieldMenu(ITEM).setTo (menu1);
    FORM_fieldMenu(RELATIVE).setTo (menu1);
    CUR_delwin(SysWin);
}
