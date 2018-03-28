/*******************************************
 **********	print.c		************
 ******************************************/

#include "Vk.h"

#include "stdcurses.h"
#include "page.h"
#include "form.h"
#include "print.d"
#include "rsInterface.h"

#define PRINTER	    (Form->field[1])
#define PRINT_N	    3
#define PRINT	    (Form->field[PRINT_N])
#define NONElabel "None"
#define NONEhelp  "None defined"

typedef struct
    {
    char *command;
    } commandInfo;

/* Forward function declarations */
PrivateFnDef void exec ();

/* Public function declarations */
PublicFnDecl char *strtok ();

/* Public variable declarations */
PublicVarDef int PRINT_Lines;
PublicVarDef PRINTER_Command PRINT_Command;
PublicVarDef MENU_Choice *printerChoices = 0;

/* Private variable declarations */
PrivateVarDef FORM *Form;
PrivateVarDef int printersDefined = FALSE;
PrivateVarDef commandInfo *printers;
PrivateVarDef commandInfo *types;
PrivateVarDef MENU_Choice *typeChoices = 0;
PrivateVarDef CHOICE_MenuChoice **PrinterChoices;
PrivateVarDef int numPrinters = 0;
PrivateVarDef int numTypes = 0;

PrivateVarDef FORM_Field formFields[] =
    {
    1, 5, CUR_A_NORMAL, 8, 0, 'a', "Printer:", 
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
    1, 14, (CUR_A_DIM | CUR_A_REVERSE), 8, 1, 'm', "        ", 
        " Use Arrow Keys To Select Printer", MENU::Reference(), NULL, 
    3, 1, CUR_A_NORMAL, 12, 0, 'a', "Print Style:",  
        static_cast<char const*>(NULL), MENU::Reference(), NULL, 
    3, 14, (CUR_A_DIM | CUR_A_REVERSE), 15, 1, 'm', "               ", 
        " Use Arrow Keys To Select Print Style", MENU::Reference(), NULL, 
    -1, 
    };


#ifdef MANUAL_PRINTERLOAD
PrivateFnDef void loadPrinters ()
{
    FILE *fp;
    char buffer[RS_MaxLine + 1];

    fp = fopen ("//irving/users/teg/print.v", "r");
    if (fp == NULL)
        return;

    while (fgets (buffer, RS_MaxLine, fp))
        {
        buffer[strlen (buffer) - 1] = '\0';
        RS_sendOnly (buffer);
        }

    fclose (fp);

    RS_compile ();

    /* wait for the prompt to continue */
    while (RS_readLine (buffer, RS_MaxLine))
        ;
}
#endif


PrivateFnDef void abortInitPrinters (int stage) {
    int i;

    /* clean-up from a bad memory allocation.  Use a CASE
       statement to drop-through each approriate stage */
    switch (stage)
        {
        case 4:
            free (types);
            /*drop-through */
        case 3:
            free (PrinterChoices);
            /*drop-through */
        case 2:
            for (i = 0; i < numPrinters; i++) {
		  free (printers[i].command);
                }
            delete[] printerChoices;
            /*drop-through */
        case 1:
            free (printers);
            /*drop-through */
        case 0:
            numPrinters = 0;
            break;
        }
}


PrivateFnDef int initPrinters () {
    char buffer[RS_MaxLine + 1];
    int *typeCount;
    int offset = 0;
    int i,
        j;

    /* set-up printers */
    /* query for number of printers */
    PRINT_Command.command[0] = '\0';
    PRINT_Command.initString[0] = '\0';

    RS_sendLine ("PrinterTools Descriptor getNumberOfPrinters");

    while (RS_readLine (buffer, RS_MaxLine) && (strcmp (buffer, "") == 0))
        ;

    if (strncmp (buffer, ">>>", 3))
        numPrinters = atoi (buffer);
    else
       numPrinters = 0;


    while (RS_readLine (buffer, RS_MaxLine))
        ;
    if (numPrinters == 0)
        return (FALSE);

    printers = (commandInfo *) calloc (numPrinters, sizeof (commandInfo));
    if (printers == NULL)
        {
        abortInitPrinters (0);
        return (FALSE);
        }
    printerChoices = new MENU_Choice[numPrinters + 1];
    if (printerChoices == NULL) {
        abortInitPrinters (1);
        return (FALSE);
    }

    /* Send query for printer info */
    RS_sendLine ("PrinterTools Descriptor displayPrinterData");

    while (RS_readLine (buffer, RS_MaxLine) && (strcmp (buffer, "") == 0))
        ;

    for (i = 0; i < numPrinters; i++) {
        printerChoices[i].label.setTo (strtok (buffer, ";"));
        printerChoices[i].help.setTo (strtok (NULL, ";"));
        printerChoices[i].letter = tolower (strtok (NULL, ";")[0]);

        printerChoices[i].handler = FORM_menuToForm;

	printerChoices[i].active = ON;

        printers[i].command = strdup (strtok (NULL, ";"));

        if (i < (numPrinters - 1))
            RS_readLine (buffer, RS_MaxLine);
    }
    while (RS_readLine (buffer, RS_MaxLine))
        ;

    /* set-up typeChoices */
    /* get count of all modes and modes per printer */
    typeCount = (int *) calloc (numPrinters, sizeof (int));
    if (typeCount == NULL)
        {
        abortInitPrinters (2);
        return (FALSE);
        }
    PrinterChoices = (CHOICE_MenuChoice **) calloc (numPrinters, sizeof (CHOICE_MenuChoice *));
    if (PrinterChoices == NULL)
        {
        abortInitPrinters (2);
        free (typeCount);
        return (FALSE);
        }

    for (i = 0; i < numPrinters; i++) {
        sprintf (buffer, "PrinterTools Descriptor Named %s getNumberOfOptions",
                          printerChoices[i].label.content ());
	RS_sendLine (buffer);

        while (RS_readLine (buffer, RS_MaxLine) && (strcmp (buffer, "") == 0))
            ;

        if (strncmp (buffer, ">>>", 3))
            typeCount[i] = atoi (buffer);
        else
            {
            numPrinters = 0;
            return (FALSE);
            }

        while (RS_readLine (buffer, RS_MaxLine))
            ;

        numTypes += typeCount[i];
        if (typeCount[i] == 0)
            numTypes++;
        }

    types = (commandInfo *) calloc (numTypes, sizeof (commandInfo));
    if (types == NULL)
        {
        abortInitPrinters (3);
        free (typeCount);
        return (FALSE);
        }
    typeChoices = new MENU_Choice[numTypes + 1];
    if (typeChoices == NULL) {
        abortInitPrinters (4);
        free (typeCount);
        return (FALSE);
    }

    for (j = 0; j < numPrinters; j++) {
        if (typeCount[j] == 0) {
            typeCount[j]++;
            PrinterChoices[j] = (CHOICE_MenuChoice *) calloc (2, 
                                              sizeof (CHOICE_MenuChoice));
            PrinterChoices[j][0].subfield = PRINT_N;
            PrinterChoices[j][0].choices = (char*)malloc (numTypes + 1);
            for (i = 0; i < numTypes; i++)
                PrinterChoices[j][0].choices[i] = '0';
            PrinterChoices[j][1].subfield = -1;

            new (&typeChoices[offset].label) V::VString (NONElabel);
            new (&typeChoices[offset].help) V::VString (NONEhelp);

            typeChoices[offset].letter = tolower (typeChoices[offset].label[0]);

            typeChoices[offset].handler = FORM_menuToForm;

            typeChoices[offset].active = ON;

            types[offset].command = (char*)malloc (1);
            types[offset].command[0] = '\0';

            PrinterChoices[j][0].choices[offset++] = '1'; 
            continue;
        }

        PrinterChoices[j] = (CHOICE_MenuChoice *) calloc (2, sizeof (CHOICE_MenuChoice));
        PrinterChoices[j][0].subfield = PRINT_N;
        PrinterChoices[j][0].choices = (char*)malloc (numTypes + 1);
        for (i = 0; i < numTypes; i++)
            PrinterChoices[j][0].choices[i] = '0';
        PrinterChoices[j][1].subfield = -1;

        sprintf (buffer, "PrinterTools Descriptor Named %s displayPrinterOptions",
                         printerChoices[j].label.content ());
	RS_sendLine (buffer);

        while (RS_readLine (buffer, RS_MaxLine) && (strcmp (buffer, "") == 0))
            ;

        for (i = 0; i < typeCount[j]; i++) {
            typeChoices[offset].label.setTo (strtok (buffer, ";"));
            typeChoices[offset].help.setTo (strtok (NULL, ";"));

            typeChoices[offset].letter = tolower (strtok (NULL, ";")[0]);

            typeChoices[offset].handler = FORM_menuToForm;

            typeChoices[offset].active = ON;

            types[offset].command = strdup (strtok (NULL, ";"));

            PrinterChoices[j][0].choices[offset++] = '1'; 

            if (i < (typeCount[j] - 1))
                RS_readLine (buffer, RS_MaxLine);
            }

        while (RS_readLine (buffer, RS_MaxLine))
            ;
    }
    free (typeCount);

    printersDefined = TRUE;

    return TRUE;
}


PublicFnDef void print (PAGE *currPage, int defaultPrinter) {
    PAGE *page;
    CUR_WINDOW *win,
               *formWin;
    int i, longest, j, currWindows, start, end;
    FORM_Field	*fptr;

/*** initialize print parameters ****/
    PRINT_Lines = 0;
    PRINT_Command.command[0] ='\0';
    PRINT_Command.initString[0] ='\0';

    MENU::Reference menu1 (new MENU (printerChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
    MENU_title(menu1) = "Printer:";

    MENU::Reference menu2 (new MENU (typeChoices, CUR_A_NORMAL, CUR_A_REVERSE, longest, i, j));
    MENU_title (menu2) = "Print Style:";

    if ((defaultPrinter >= 0) && (defaultPrinter < numPrinters))
        {
	formFields[1].attr = CUR_A_NORMAL;
	formFields[1].type = 'a';
	strcpy (formFields[1].value, printerChoices[defaultPrinter].label);
        formFields[1].help = NULL;	

    	for (
	    i = 0;
	    (size_t)i < strlen (PrinterChoices[defaultPrinter][0].choices);
	    i++
	) MENU_choiceActive (
	    menu2, i
	) = PrinterChoices[defaultPrinter][0].choices[i] == '0' ? OFF : ON;

        i = 0;
        while (PrinterChoices[defaultPrinter][0].choices[i] == '0')
            i++;

    	MENU_currChoice (menu2) = i;

	FORM_makeForm (Form, formFields, i);
        }
    else
        {
	formFields[1].attr = (CUR_A_DIM | CUR_A_REVERSE);
	formFields[1].type = 'm';	
	strcpy (formFields[1].value, "        ");
	formFields[1].help = " Use Arrow Keys to Select Printer";	

	FORM_makeForm (Form, formFields, i);
	FORM_fieldMenu (PRINTER) = menu1;
	FORM_fieldChoiceArray (PRINTER) = PrinterChoices;
        }

    FORM_fieldMenu (PRINT) = menu2;

/*** create form window ****/
    win = CUR_newwin (7, 32, 10, 1);
    WIN_ReverseBox (win, NULL);
    CUR_wmove (win, 0, 11);
    CUR_wattron (win, CUR_A_REVERSE);
    CUR_wprintw (win, "PRINT FORM");
    CUR_wmove (win, CUR_WIN_maxy (win), 2);
    CUR_wprintw (win, "Execute(F2)  Quit(F9)");
    CUR_wattroff(win, CUR_A_REVERSE);
    formWin = CUR_subwin (win, 5, 30, 11, 2);

/**** create page object ****/
    currWindows = PAGE_windowCount (currPage);
    PAGE_createPage (page, (currWindows + 2), NULL, NULL, NULL, PAGE_noType, i);
    PAGE_fkey (page, 1) = exec;
    PAGE_status (page) = PAGE_ExitOnExec;
    
    for (i = 0; i < currWindows; i++)
        {
	PAGE_createElement (page, i, NULL, 
	    PAGE_window (currPage, i), PAGE_Init, NULL, FALSE);
        }
    PAGE_createElement (page, currWindows, NULL, win, PAGE_Init, NULL, FALSE);
    PAGE_createElement (page, currWindows + 1, Form, formWin,
				 PAGE_Input, FORM_handler, TRUE);
    
    PAGE_handler (page);

/*** cleanup ***/
    free (Form);
    CUR_delwin (formWin);
    CUR_delwin (win);
    PAGE_deletePage (page, i);
}


#ifdef OLD_PRINTER
/***********************************************
 **********	Execute		****************
 ***********************************************/
PrivateFnDef char *getPrintStyle(char printer) {
    MENU *menu = FORM_fieldMenu (PRINT);
    
    if (printer == 's')
        {
	switch (MENU_choiceLetter(menu, MENU_currChoice(menu)))
            {
	case 'p':	    /**** pica ****/
	    return("10");

	case 'e':	    /**** elite ****/
	    return("12");

	case 'q':	    /**** correspondence quality ****/
	    return("q");

	case 'c':	    /**** compressed ****/
	    return("c");

	case 'l':	    /**** expanded ****/
	    return("e");

	default:
	    
	    break;
	}
	
    }
    else
    {
	switch (MENU_choiceLetter(menu, MENU_currChoice(menu)))
	{
	case 'p':	  
	    return("prestige");

	case 'b':	
	    return("bold");

	case 'c':	 
	    return("courier");

	case 'i':	 
	    return("italic");

	default:
	    return(" ");
	}
	
    }
    return(" ");
}
#endif

PrivateFnDef void exec () {
    char *mode;
    int  i;

    PRINT_Lines = 66;	/*** default for portrait mode ***/
    
#ifdef OLD_PRINTER
    if (0 == strncmp(FORM_fieldValue(PRINTER), " Laser", 6))	
        {
        if (0 == strncmp(FORM_fieldValue(MODE), " Landscape", 10))
            {
            mode = "landscape";
/****	    PRINT_Lines = 51;	    laser spacing problem ****/
            }
        else
            mode = "portrait";
        sprintf (PRINT_Command, "lp -s -dlaser -o%s -o%s", 
						getPrintStyle('l'), 
						mode);
        }
    else
        if (0 == strncmp (FORM_fieldValue (PRINTER), " System", 7))	
            {
            sprintf (PRINT_Command, "lp -s -o%s", getPrintStyle('s'));
            }
        else
            sprintf (PRINT_Command, "pcprint");
#endif
    /* lookup the printer command associated with the chosen printer */
    i = 0;
    while ((strcmp (FORM_fieldValue (PRINTER), printerChoices[i].label) != 0) &&
            i < numPrinters)
        i++;
    if (i < numPrinters)
        strcpy (PRINT_Command.command, printers[i].command);

    /* lookup the type command associated with the chosen type */
    i = 0;
    while ((strcmp (FORM_fieldValue (PRINT), typeChoices[i].label) != 0) &&
            i < numTypes)
        i++;
    if (i < numTypes)
        strcpy (PRINT_Command.initString, types[i].command);
}

PublicFnDef void cleanupPrinters () {
    int i;
    char  fname[128];

    if (numPrinters == 0) {
        delete[] printerChoices;
        return;
    }

    /* if the printers were used,  it is likely that a file
       called /tmp/vision(PID) exists.  If it does,  then it
       is equally as likely that /tmp/viserr(PID) exists.
       Nuke 'em */
    sprintf (fname, "/tmp/vision%d", getpid ());
    if (access (fname, 0) == 0) {
        remove (fname);
        sprintf (fname, "/tmp/viserr%d", getpid ());
        remove (fname);
    }

    for (i = 0; i < numPrinters; i++) {
        free (printers[i].command);

        free (PrinterChoices[i][0].choices);
        free (PrinterChoices[i]);
        }
    free (printers);
    delete[] printerChoices;
    free (PrinterChoices);

    for (i = 0; i < numTypes; i++)
        {
        free (types[i].command);
        }
    delete[] typeChoices;
    free (types);
}

PublicFnDef int checkPrinters () {
    char buffer[RS_MaxLine + 1];

    if (printersDefined)
        return (TRUE);

    if (!initPrinters ()) {
        printerChoices = new MENU_Choice[2];

        printerChoices[0].label.setTo (NONElabel);
        printerChoices[0].help.setTo (NONEhelp);
        printerChoices[0].letter = tolower (printerChoices[0].label[0]);
        printerChoices[0].handler = FORM_menuToForm;
        printerChoices[0].active = ON;
    }

    return (numPrinters);
}
