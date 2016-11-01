
/*****************************************************************************
 * Copyright (C) Ajinkya Pralhad Wandale ajinkyawandale20@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include<ncurses.h>
#include<menu.h>
#include<form.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<ctype.h>
#include<time.h>
#define size(a) ( sizeof(a) / sizeof(a[0]) )
MENU *gmenu;
ITEM **gitem;
int match;
/*function to print the list of previous invoices on the screen*/
void print_invoice_list(WINDOW *win, char **arr, int start, int height, int color) {
	int i = start;
	init_pair(10, COLOR_BLACK, COLOR_MAGENTA);
	for(;i < height + 1;i++) {
		if(i == color)
			wattron(win, COLOR_PAIR(10));
		mvwprintw(win, i + 4, 1, "%s", arr[i - 1]);
		if(i == color)
			wattroff(win, COLOR_PAIR(10));
	}
	wclrtobot(win);
	box(win, 0, 0);
}
/*function to restore an item in the database if the invoice is not saved or printed*/
void restore_items(int i, char **arr) {
	FILE *fp;
	char str[280];
	int count = 0, qty = 0;
	fp = fopen("database.csv", "r+");
	if(fp == NULL)
		return;
	for(;count < i;count++) {
		fseek(fp, 0, SEEK_SET);
		while(fgets(str, 280, fp) != NULL) {
			if(strncmp(arr[count], str, 30) == 0) {
				str[247] = '\0';
				arr[count][247] = '\0';
				qty = atol(&str[217]) + atol(&arr[count][217]);
				fseek(fp, -62, SEEK_CUR);
				fprintf(fp, "%-30d", qty);
				break;
			}
		}
	}
	fclose(fp);
	for(count = 0;count < i;count++)
		free(arr[count]);
}
/*function to add an item in the invoice*/
char* add_to_invoice(char *str1, char *str2) {
	char *str, arr[31];
	int i, qty;
	float price;
	FILE *fp;
	fp = fopen("database.csv", "r+");
	if(fp == NULL)
		return NULL;
	str = (char *) malloc(sizeof(char) * 280);
	while(fgets(str, 280, fp) != NULL) {
		if(strncmp(str, str1, 30) == 0) {
			strncpy(arr, &str[217], 30);
			qty = atol(str2);
			price = atof(&str[248]);
			i = atol(arr) - qty;
			fseek(fp, -62, SEEK_CUR);
			fprintf(fp, "%-30d", i);
			sprintf(&str[217], "%-30d,%-30.3f", qty, 1.0 * (qty * price)); 
			break;
		}
	}
	fclose(fp);
	return str;
}
/*function to check available quantitiy of medicine in stock*/
int check_quantity(char *field) {
	char str[280], arr[31];
	int qty = 0;
	FILE *fp;
	fp = fopen("database.csv", "r");
	if(fp == NULL)
		return -1;
	while(fgets(str, 280, fp) != NULL) {
		if(strncmp(str, field, 30) == 0) {
			strncpy(arr, &str[217], 30);
			qty += atol(arr); 
		}
	}
	fclose(fp);
	return qty;
}
void swap_fore(int x, char *str) {
	int i = strlen(str);
	while(x <= i) {
		str[x - 1] = str[x];
		x++;
	}
}
void swap_back(int x, char *str) {
	int i = strlen(str);
	while( i >= x) {
		str[i + 1] = str[i];
		i--;
	} 
}
void tokens(char *str, char *delim, char **token) {
	int i = 0;
	token[i] = strtok(str, delim);
	while( token[i] != NULL) {
		i++;
		token[i] = (char*)malloc(31 * sizeof(char));
		token[i] = strtok(NULL, delim);
	}
}
/*function to search in the database for the key*/
WINDOW** search_inventory(const char *name, char *text, int *location) {
	int i = 0, k, size = strlen(text), len;
	char **token, str[280], *delim = ",\n";
	WINDOW **dpad;
	init_pair(70, COLOR_WHITE, COLOR_MAGENTA);
	FILE *fp;
	match = 0;
	fp = fopen("database.csv", "r");
	if(fp == NULL) {
		return NULL;
	}
	if(strcmp(name, "Name") == 0)
		k = 0;
	else if(strcmp(name, "Category") == 0)
		k = 3;
	else if(strcmp(name, "Batch No") == 0)
		k = 4;
	else if(strcmp(name, "Location") == 0)
		k = 5;
	else if(strcmp(name, "Expiry Date") == 0)
		k = 6;
	else if(strcmp(name, "Quantity") == 0)
		k = 7;
	
	token = (char**)malloc(10 * sizeof(char*));
	
	while(fgets(str, 280, fp) != NULL) {
		i = 0;
		tokens(str, delim, token);
		
		if(strncasecmp(token[k], text, size) == 0) {
			if( match == 0)
				dpad = (WINDOW **) malloc(sizeof(WINDOW *));
			len = size(dpad);
			if(len - 1 < match)
				dpad = (WINDOW **) realloc(dpad, sizeof(WINDOW *) * (match + 1));
			dpad[match] = newpad(1, 150);
			wattron(dpad[match], COLOR_PAIR(70));
			while( token[i] != NULL) {
				mvwaddstr(dpad[match], 0, *(location + i) - 7, token[i]);
				i++;
			}
			wattroff(dpad[match], COLOR_PAIR(70));
			match++;
		}
	}
	free(token);
	fclose(fp);
	return dpad;
}
/*function to add a new item in the database*/
int newitem_process(FORM *form, FIELD **field) {
	int j = 0, i = field_count(form);
	fpos_t position;
	char **token, str[280], *delim = ",\n";
	
	FILE *fp;
	fp = fopen("database.csv", "r+");
	if(fp == NULL) { 
		fp = fopen("database.csv", "w+");	
	}
	token = (char**)malloc(10 * sizeof(char*));
	
	fgetpos(fp, &position);
	while(fgets(str, 280, fp) != NULL) {
		tokens(str, delim, token);
		if( !strcmp(token[0], field_buffer(field[0], 0)) &&  !strcmp(token[1], field_buffer(field[1], 0)) && !strcmp(token[4], field_buffer(field[4], 0)) && !strcmp(token[6], field_buffer(field[6], 0)) ) 
			break;
		fgetpos(fp, &position);
	}

	fsetpos(fp, &position);
	j = 0;
	while(j < (i - 1) ) {
		fprintf(fp, "%30s", field_buffer(field[j], 0));
		if(j <  (i - 2) )
			fprintf(fp, ",");
		else 
			fprintf(fp, "\n");
		j++;
	}
	fclose(fp);
	return 1;
}
/*function to set buffers when item name is selected to update or remove*/
int set_buffer(FIELD **field, ITEM *item) {
	char str[280];
	FILE *fp;
	int i = item_count(gmenu), j, repeat = 0;
	for(j = 0;j < i;j++) {
		if(strcmp(item_name(item), item_name(gitem[j])) == 0) {
			if(gitem[j] != item)
				repeat++;
			else
				break; 
		}
	}
	fp = fopen("database.csv", "r");
	if(fp == NULL)
		return -1;
	j = repeat;
	while(fgets(str, 280, fp) != NULL) {
		if (strncmp(item_name(item), str, strlen(item_name(item))) == 0) {
			if(j > 0)
				j--;
			else {
				for(i = 1;i < 9;i++) {
					str[30 * i + i + 30] = '\0';
					set_field_buffer(field[i], 0, &str[30 * i + i]);
				}
				return repeat;
			}
		}
	}
	return -1;
}
/*function to remove item from database*/
void remove_item(FIELD **field, int repeat) {
	FILE *fp, *fd;
	char str[280];
	fp = fopen("database.csv", "r");
	if(fp == NULL)
		return;
	fd = fopen("tmp.csv", "w");
	if(fd == NULL)
		return;
	while(fgets(str, 280, fp) != NULL) {
		if( !strncmp(field_buffer(field[0], 0), str, 30) ) {
			if(repeat > 0)
				repeat--;
			else {
				continue;
			}
		}
		fprintf(fd, "%s", str);
	}
	fclose(fp);
	fclose(fd);
	remove("database.csv");
	rename("tmp.csv", "database.csv");
}
/*function to print matched item names on screen*/
void print_inventory(WINDOW **dpad, WINDOW *win, int start, int height) {
	int i = 0, j = 17, k;
	k = (match < height)? match : height;
	while( i < k) {
		mvwprintw(win, j - 9, 3, "%2d", start + 1);
		prefresh(dpad[start], 0, 0, j, 9, j + 1, 139);
		j++;
		i++;
		start++;
	}
	copywin(stdscr, win, 17, 9, 9, 7, match, 139, FALSE);
	wrefresh(win);
}
/*function to return current invoice nember*/
int invoice_number(void) {
	char a[11];
	FILE *fp;
	fp = fopen("Invoice.csv", "r");
	if(fp == NULL)
		return 1;
	fgets(a, 10, fp);
	fclose(fp);
	return (atoi(a) + 1);
}
/*function to save a bill in the file*/
void save_bill(int i, char **arr, FIELD **field, float net_amount) {
	FILE *fp;
	int j, k, index[] = {0, 31, 124, 186, 217, 248};;
	char a[11];
	time_t t = time(0);
	struct tm *time = localtime( & t );
	
	fp = fopen("Invoice.csv", "r+");
	if(fp == NULL) {
		fp = fopen("Invoice.csv", "w");
		fprintf(fp, "%-10d\n", 0);
		fseek(fp, 0, SEEK_SET);
	}
	fgets(a, 10, fp);
	fseek(fp, 0, SEEK_SET);
	fprintf(fp, "%-10d", 1 + atoi(a));
	fseek(fp, 0, SEEK_END);
	fprintf(fp, "%-10d,%s,%s,%s\n", 1 + atoi(a), field_buffer(field[0], 0), field_buffer(field[1], 0), field_buffer(field[2], 0));
	fprintf(fp, "%-2d,%-10.3lf,%2d/%2d/%4d\n", i, net_amount, time->tm_mday,time->tm_mon,1900 + time->tm_year);
	for(j = 0;j < i;j++) {
		for(k = 0;k < size(index) - 1;k++)
			fprintf(fp, "%s,", &arr[j][index[k]]);
		fprintf(fp, "%s\n", &arr[j][index[k]]);	
	}
	fclose(fp);
}
/*fucntion to display all items in the invoice on the screen*/
void print_invoice_on_screen(char **arr, int i, WINDOW *win, int *location, int color) {
	int index[] = {0, 31, 124, 186, 217, 248};
	int j, k;
	init_pair(10, COLOR_BLACK, COLOR_MAGENTA);
	
	for(j = 0;j < 9;j++)
		arr[i - 1][30 * j + j + 30] = '\0';
	for(j = 0;j < i;j++) {
		if( j == color - 1)
			wattron(win, COLOR_PAIR(10));
		mvwprintw(win, j + 11, 1, "%d", j + 1);
		for(k = 0;k < size(index);k++)
			mvwprintw(win, j + 11, location[k], "%s", &arr[j][index[k]]);
		if( j == color - 1)
			wattroff(win, COLOR_PAIR(10));
	}
	wmove(win, j + 11, 1);
	wclrtoeol(win);
	box(win, 0, 0);
}
/*function to print the bill in a text file*/
void print_bill(WINDOW *win, int i, char **arr, FIELD **field, int invoice_no) {
	FILE *fp, *fd;
	int j ,w = 80, qty, k;
	float amount, net_amount = 0;
	char str1[10], str2[50] = "bill_", str3[150];
	sprintf(str1, "%d", invoice_no);
	strcat(str2, str1);
	strcat(str2, ".txt");
	fp = fopen(str2, "w");
	if(fp == NULL)
		return;
	fd = fopen("profile.csv", "r");
	if(fd == NULL) {
		mvwprintw(win, 30, 2, "No profile available to print the bill");
		return;
	}
	for(j = 0;j < ( w - 7 ) / 2;j++)
		fputc(' ', fp);
	fputs("Invoice\n\n", fp);
	
	fseek(fd, 51, SEEK_SET);
	fgets(str3, 50, fd);
	fputs(str3, fp);
	fputc('\n', fp);
	fseek(fd, 102, SEEK_SET);
	fgets(str3, 50, fd);
	fputs(str3, fp);
	fputc('\n', fp);
	for(j = 0;j < w;j++)
		fputc('-', fp);
	fputc('\n', fp);
	fprintf(fp, "Patient's Name :- %s", field_buffer(field[0], 0));
	for(j = 0;j < 14;j++)
		fputc(' ', fp);
	time_t t = time(0);
	struct tm *time = localtime( &t );
	fprintf(fp, "Date :- %d-%d-%d \n",time->tm_mday,time->tm_mon,1900 + time->tm_year);
	fprintf(fp, "Age :- %s ", field_buffer(field[1], 0));
	for(j = 0;j < 49;j++)
		fputc(' ', fp);
	fprintf(fp, "Invoice No :- ");
	fprintf(fp, "%d", invoice_no);
	fputc('\n', fp);
	for(j = 0;j < w;j++)
		fputc('-', fp);
	fputc('\n', fp);
	for(j = 0;j < w;j++)
		fputc('-', fp);
	fputc('\n', fp);
	fprintf(fp, "Sr. No. | %30s | %6s | %10s | %10s\n", "Description", "Qty", "Rate", "Amount");
	for(j = 0;j < w;j++)
		fputc('-', fp);
	fputc('\n', fp);
	for(k = 0;k < i;k++) {
		fprintf(fp, "%7d | ", k  + 1);
		arr[k][30] = '\0';
		fprintf(fp, "%s | ", arr[k]);
		arr[k][247] = '\0';
		qty = atol(&arr[k][217]);
		amount = atof(&arr[k][248]);
		fprintf(fp, "%6d | ", qty);
		fprintf(fp, "%10f | ", 1.0 * ( amount / qty));
		fprintf(fp, "%10f \n", amount);	
		net_amount += amount;
	}
	for(j = 0;j < w;j++)
		fputc('-', fp);
	fputc('\n', fp);
	for(j = 0;j < w - 28;j++)
		fputc(' ', fp);
	fprintf(fp, "Net Amount :- %10f\n", net_amount);
	for(j = 0;j < w;j++)
		fputc('-', fp);
	fputc('\n', fp);
	fclose(fp);
	fclose(fd);
	strcpy(str3, "libreoffice --convert-to pdf ./");
	strcat(str3, str2);
	strcat(str3, " >/dev/null 2>&1");
	system(str3);
	napms(1500);
	remove(str2);
	j = strlen(str2);
	strcpy(&str2[j - 3], "pdf");
	strcpy(str3, "evince ");
	strcat(str3, str2);
	strcat(str3, " &  >/dev/null 2>&1");
	system(str3);
	mvwprintw(win, 30, 3, "Bill printed successfully");
}
/*function to read previous invoices form file*/
char** read_invoices(int *location, WINDOW *win){
	FILE *fp;
	int size, j, k;
	char **arr, str[31];
	fp = fopen("Invoice.csv", "r");
	if(fp == NULL) {
		arr = 	(char**)malloc(sizeof(char*));
		arr[0] = NULL;
		return arr;
	}
	fgets(str, 11, fp);
	fgetc(fp);
	size = atoi(str);
	arr = (char**)malloc((size + 1) * sizeof(char*));
	if(arr == NULL)
		return NULL;
	
	for(j = 0;j < size;j++) {
		arr[j] = (char *) malloc(115 * sizeof(char));
		fgets(str, 11, fp);
		sprintf(arr[j], "%-11s", str);
		fgetc(fp);
		
		fgets(str, 31, fp);
		sprintf(&arr[j][location[1] -1], "%-31s", str);
		fgetc(fp);
			
		fgets(str, 4, fp);
		sprintf(&arr[j][location[2] -1], "%-5s", str);
		fgetc(fp);
		
		fgets(str, 31, fp);
		sprintf(&arr[j][location[3] -1], "%-31s", str);
		fgetc(fp);
		
		fgets(str, 3, fp);
		sprintf(&arr[j][location[4] -1], "%-16s", str);
		fgetc(fp);
		k = atoi(str);
		
		fgets(str, 11, fp);
		sprintf(&arr[j][location[5] -1], "%-11s", str);
		fgetc(fp);
		
		fgets(str, 11, fp);
		sprintf(&arr[j][location[6] -1], "%-s", str);
		fgetc(fp);
		fseek(fp, 186 * k, SEEK_CUR);
	}
	arr[j] = (char *)NULL;
	return arr;
}
/*function to display privious invoices*/
void previous_invoice(WINDOW *win) {
	char *menu_names[] = {"Invoice No", "Patients's Name", "Age", "Doctor's Name", "Items Purchased", "Net Amount", "Date"};
	char **data;
	int x, y, i, location[] = {1, 12, 43, 48, 79, 95, 106}, count = 0, height, color = 1, start = 1;
	getmaxyx(win, y, x);
	mvwaddch(win, 2, x - 1, ACS_RTEE);
	mvwaddch(win, 2, 0, ACS_LTEE);
	mvwhline(win, 2, 1, ACS_HLINE, x - 2);
	mvwhline(win, 4, 1, ACS_HLINE, x - 2);
	for(i = 0;i < size(menu_names);i++) {
		mvwprintw(win, 3, location[i], "%s", menu_names[i]);
		if(i < size(menu_names) - 1) {
			mvwaddch(win, 3, location[i + 1] - 1, ACS_VLINE);
			mvwaddch(win, 4, location[i + 1] - 1, ACS_BTEE);
			mvwaddch(win, 2, location[i + 1] - 1, ACS_TTEE);
		}
	}
	data = read_invoices(location, win);
	while(data[count] != NULL)
		count++;
	if(count == 0)
		mvwprintw(win, y - 2, 2, "No invoices to display");
	wrefresh(win);
	height = ( (y - 6) < count) ? (y - 6) : count;
	print_invoice_list(win, data, start, height, color);
	
	while( (i = wgetch(win)) ) {
		switch(i) {
			case KEY_DOWN:
				if(color < count)
					color++;
				if( (count - start) > (y - 6) )
					start++;
				print_invoice_list(win, data, start, height, color);
				
				break;
			case KEY_UP:
				if(count > (y - 6) && start > 1)
					start--;
				if(color > 1)
					color--;
				print_invoice_list(win, data, start, height, color);
				break;
			case 27:
				mvwaddch(win, 2, x - 1, ACS_RTEE);
				mvwaddch(win, 2, 0, ACS_LTEE);
				mvwhline(win, 2, 1, ACS_HLINE, x - 2);
				wmove(win, 3, 1);
				wclrtobot(win);
				wrefresh(win);
				return;
				break;
		}
	}
	wrefresh(win);
}
/*function to create the dropbox */
WINDOW* medicine_name(WINDOW *mainwin, WINDOW *win, FIELD *field, int count, int y, int x) {
	char token[31], str[280], **data;
	int i = 0, j = 0, k;
	FILE *fp;
	fp = fopen("database.csv", "r");
	if(fp == NULL)
		return NULL;
	data = (char**) malloc(10 * sizeof(char*));	
	while(fgets(str, 280, fp) != NULL) {
		strcpy(token, strtok(str, ","));
		if(strncasecmp(token, field_buffer(field, 0), count) == 0) {
			if( i % 10 == 0 )
				data = (char **) realloc(data, ( i + 10 ) * sizeof(char *));
			data[i] = (char *) malloc(31 * sizeof(char));
			strcpy(data[i], token);
			i++;
		}
	}
	if( i == 0 && win == NULL)
		return win;
	if(win != NULL) {
		unpost_menu(gmenu);
		free_menu(gmenu);
		j = 0;
		while(gitem[j] != NULL)
			free_item(gitem[j++]);
		free_item(gitem[j]);
		free(gitem);
		wbkgd(win, COLOR_PAIR(3));
		wclear(win);
		delwin(win);
		if( i == 0)
			return NULL;
	}
	k = (i > 4)? 4 : i ; 
	win = derwin(mainwin, k, 30, y, x);
	wbkgd(win, COLOR_PAIR(43));
	gitem = (ITEM **) malloc( (i + 1) * sizeof(ITEM *) );
	for(j = 0;j < i;j++)
		gitem[j] = new_item(data[j], "");
	free(data);
	gitem[j] = NULL;
	gmenu = new_menu(gitem);
	set_menu_win(gmenu, win);
	set_menu_format(gmenu, k, 1);
	post_menu(gmenu);
	wrefresh(win);
	return win;
}
/*function to display the new invoice and displaying it on the screen*/
void newinvoice(WINDOW *win) {
	FIELD **field;
	FORM *form;
	WINDOW *win1, *swin = NULL;
	int i, count = 0, x, y, location[] = {9, 39, 59, 74, 89, 104}, qty = 0, bill_items = 0, restore = 1, tab = 0, save = 0, write = 0;
	int invoice_no;
	char *menu_names[] = {"Medicine Name", "Company", "Batch No", "Expiry", "Quantity", "Amount"};
	char **bill_data = NULL, *emptystr= "                              ", *arr;
	float  net_amount = 0;
	
	field = (FIELD **)malloc(9 * sizeof(FIELD *));
	field[0] = new_field(1, 30, 0, 1, 0, 0);
	field[1] = new_field(1, 3, 0, 33, 0, 0);
	field[2] = new_field(1, 30, 0, 38, 0, 0);
	field[3] = new_field(1, 30, 2, 1, 0, 0);
	field[4] = new_field(1, 5, 2, 33, 0, 0);
	field[5] = new_field(1, 5, 2, 41, 0, 0);
	field[6] = new_field(1, 11, 2, 48, 0, 0);
	field[7] = new_field(1, 12, 2, 61, 0, 0);
	field[8] = NULL;
	for(i = 0;i < 5;i++) {
		field_opts_off(field[i], O_AUTOSKIP);
		set_field_back(field[i], COLOR_PAIR(43));
		set_field_fore(field[i], COLOR_PAIR(43));
	} 
	for(i = 5;i < 8;i++) {
		field_opts_off(field[i], O_AUTOSKIP | O_EDIT);
		set_field_back(field[i], COLOR_PAIR(42));
		set_field_fore(field[i], COLOR_PAIR(42));
	}
	set_field_buffer(field[5], 0, " Add ");
	set_field_buffer(field[6], 0, " Save Bill ");
	set_field_buffer(field[7], 0, " Print Bill ");
	form = new_form(field);
	
	win1 = derwin(win, 3, 80, 4, 1);		
	set_form_win(form, win);
	set_form_sub(form, win1);
	post_form(form);
	mvwprintw(win, 3, 2, "Patient's Name*");
	mvwprintw(win, 3, 34, "Age*");
	mvwprintw(win, 3, 39, "Doctor's Name*");
	mvwprintw(win, 5, 2, "Medicine Name");
	mvwprintw(win, 5, 34, "Qty");
	mvwprintw(win, 3, 71, "Invoice No");
	invoice_no = invoice_number();
	mvwprintw(win, 4, 73, "%d", invoice_no);
	getmaxyx(win, y, x);
	mvwhline(win, 7, 1, ACS_HLINE, x - 2);
	mvwaddch(win, 7 , 0, ACS_LTEE);
	mvwaddch(win, 7, x - 1, ACS_RTEE);
	mvwhline(win, 9, 1, ACS_HLINE, x - 2);
	mvwaddch(win, 9 , 0, ACS_LTEE);
	mvwaddch(win, 9, x - 1, ACS_RTEE);
	mvwhline(win, y - 3, 1, ACS_HLINE, x - 2);
	mvwaddch(win, y - 3 , 0, ACS_LTEE);
	mvwaddch(win, y - 3, x - 1, ACS_RTEE);
	mvwhline(win, y - 5, 1, ACS_HLINE, x - 2);
	mvwaddch(win, y - 5 , 0, ACS_LTEE);
	mvwaddch(win, y - 5, x - 1, ACS_RTEE);
	mvwprintw(win, y - 4, 90, "Net Amount");
	for(i = 0;i < size(location);i++)
		mvwprintw(win, 8, location[i], "%s", menu_names[i]);
	wrefresh(win);
	
	form_driver(form, REQ_FIRST_FIELD);
	while( (i = wgetch(win) )) {
		if(isprint(i) || (i == ' ')) {
			wmove(win, 30, 1);
			wclrtoeol(win);
			form_driver(form, i);
			form_driver(form, REQ_END_LINE);
			if(current_field(form) == field[3]) {
				count++;
				swin = medicine_name(win, swin, field[3], count, 7, 2);
			}
			else if(current_field(form) == field[4] && strcmp(emptystr, field_buffer(field[3], 0)) ) {
				qty = check_quantity(field_buffer(field[3], 0));
				getyx(win, y, x);
				wattron(win, COLOR_PAIR(43));
				mvwprintw(win, 7, 34, "%5d", qty);
				wattroff(win, COLOR_PAIR(43));
				wmove(win, y, x);
				write = -1;
			}
		}
		else {
			switch(i) {
				case KEY_DOWN:
					if(tab > 0 && tab < bill_items) {
						tab++;
						print_invoice_on_screen(bill_data, bill_items, win, location, tab);
					}
					else if(current_field(form) == field[3] && swin != NULL) {
						menu_driver(gmenu, REQ_DOWN_ITEM);
						wrefresh(swin);
					}
					else {
						form_driver(form, REQ_NEXT_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				case KEY_UP:
					if(tab > 1) {
						tab--;
						print_invoice_on_screen(bill_data, bill_items, win, location, tab);
					}
					else if(current_field(form) == field[3] && swin != NULL) {
						menu_driver(gmenu, REQ_UP_ITEM);
						wrefresh(swin);
					}
					else {
						form_driver(form, REQ_PREV_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				case KEY_NPAGE:
					if(current_field(form) == field[3] && swin != NULL) {
						menu_driver(gmenu, REQ_SCR_DPAGE);
						wrefresh(swin);
					}
					break;
				case KEY_PPAGE:
					if(current_field(form) == field[3] && swin != NULL) {
						menu_driver(gmenu, REQ_SCR_UPAGE);
						wrefresh(swin);
					}
					break;
				case 9:
					if(tab == 0)
						tab = bill_items;
					else
						tab = 0;
					break;
				case KEY_LEFT:
					form_driver(form, REQ_PREV_CHAR);
					break;
				case KEY_RIGHT:
					form_driver(form, REQ_NEXT_CHAR);
					break;
				case 27:
					if(current_field(form) == field[3] && swin != NULL) {
						set_field_buffer(field[3], 0, "");
						goto deletewin;
					}
					else {
						if(bill_items > 0 && restore == 1)
							restore_items(bill_items, bill_data);
						free(bill_data);
						goto end;
					}
						break;
				case KEY_BACKSPACE: case 127:
					form_driver(form, REQ_DEL_PREV);
					form_driver(form, REQ_END_LINE);
					if(current_field(form) == field[3] && count > 0) {
						count--;
						if(count > 0)	
							swin = medicine_name(win, swin, field[3], count, 7, 2);
					}
					break;
				case KEY_DC:
					if(tab > 0) {
						restore_items(1, &bill_data[tab - 1]);
						net_amount -= atof(&bill_data[tab - 1][248]);
						getmaxyx(win, y, x);
						mvwprintw(win, y - 4, location[5], "%-10.3f", net_amount);
						bill_data[tab - 1] = bill_data[bill_items - 1];
						bill_items--;
						bill_data = (char **) realloc(bill_data, (bill_items) * sizeof(char*));
						if(bill_items > 0) {
							tab = 1;
							print_invoice_on_screen(bill_data, bill_items, win, location, tab);
						}
						if(bill_items == 0) {
							tab = 0;
							wmove(win, 11, 1);
							wclrtoeol(win);	
						}
						wrefresh(win);
					}
					else {
						form_driver(form, REQ_DEL_CHAR);
						form_driver(form, REQ_END_LINE);
						if(current_field(form) == field[3] && count > 0) {
							count--;
							if(count > 0)
								swin = medicine_name(win, swin, field[3], count, 7, 2);
						}
					}
					break;
				case 10:
					getmaxyx(win, y, x);
					wmove(win, y - 2, 1);
					wclrtoeol(win);
					if(current_field(form) == field[7]) {
						getmaxyx(win, y, x);
						if(bill_items == 0) {
							mvwprintw(win, y - 2, 2, "No items to print");
							form_driver(form, REQ_FIRST_FIELD);
						}
						else if(strcmp(field_buffer(field[0], 0), emptystr) == 0) {
							mvwprintw(win, y - 2, 2, "Field Patient's Name cannot be empty");
							form_driver(form, REQ_FIRST_FIELD);	
						}
						else if(strcmp(field_buffer(field[2], 0), emptystr) == 0) {
							mvwprintw(win, y - 2, 2, "Field Doctor's Name cannot be empty");
							form_driver(form, REQ_UP_FIELD);	
						}
						else if(strcmp(field_buffer(field[1], 0), "   ") == 0) {
							mvwprintw(win, y - 2, 2, "Field Patient's Age cannot be empty");
							form_driver(form, REQ_UP_FIELD);
							form_driver(form, REQ_PREV_FIELD);		
						}
						else {
							print_bill(win, bill_items, bill_data, field, invoice_no);
						}
					}
					if(current_field(form) == field[6] && save == 0) {
						getmaxyx(win, y, x);
						if(bill_items == 0) {
							mvwprintw(win, y - 2, 2, "No items to save");
							form_driver(form, REQ_FIRST_FIELD);
						}
						else if(strcmp(field_buffer(field[0], 0), emptystr) == 0) {
							mvwprintw(win, y - 2, 2, "Field Patient's Name cannot be empty");
							form_driver(form, REQ_FIRST_FIELD);
						}
						else if(strcmp(field_buffer(field[2], 0), emptystr) == 0) {
							mvwprintw(win, y - 2, 2, "Field Doctor's Name cannot be empty");
							form_driver(form, REQ_UP_FIELD);		
						}
						else if(strcmp(field_buffer(field[1], 0), "   ") == 0) {
							mvwprintw(win, y - 2, 2, "Field Patient's Age cannot be empty");
							form_driver(form, REQ_UP_FIELD);
							form_driver(form, REQ_PREV_FIELD);		
						}
						else {
							save_bill(bill_items, bill_data, field, net_amount);
							restore = 0;
							save = 1;
							mvwprintw(win, y - 2, 2, "Bill Saved Successfully");
						}
					}
					else if(current_field(form) == field[3]) {
						if(swin != NULL) {
							set_field_buffer(field[3], 0, item_name(current_item(gmenu)) );
							arr = field_buffer(field[3], 0);
							count = strlen(arr);
							while(arr[count - 1] == ' ')
								count--;
							goto deletewin;
						}
					}
					else if(current_field(form) == field[5]) {
						if( (strcmp(field_buffer(field[3], 0), emptystr) != 0) && (atol(field_buffer(field[4], 0)) <= qty ) && (atol(field_buffer(field[4], 0)) > 0 ) ) {
							if(bill_items == 0)
								bill_data = (char **) malloc(sizeof(char*));
							else
								bill_data = (char **) realloc(bill_data, (bill_items + 1) * sizeof(char*));
							bill_items++;
							bill_data[bill_items - 1] = add_to_invoice(field_buffer(field[3], 0), field_buffer(field[4], 0));
							print_invoice_on_screen(bill_data, bill_items, win, location, bill_items);
							net_amount += atof(&bill_data[bill_items - 1][248]);
							getmaxyx(win, y, x);
							mvwprintw(win, y - 4, location[5], "%-10.3f", net_amount);
							set_field_buffer(field[3], 0, "");
							count = 0;
							set_field_buffer(field[4], 0, "");
							form_driver(form, REQ_PREV_FIELD);
							form_driver(form, REQ_PREV_FIELD);
						}
						else {
							getmaxyx(win, y, x);
							mvwprintw(win, y - 2, 2, "Enter correct quantity or medicine name");
							form_driver(form, REQ_PREV_FIELD);
							form_driver(form, REQ_PREV_FIELD);
						}
					}
					break;
			}
		}
		if(current_field(form) != field[4] && write == -1) {
			getyx(win, y, x);
			mvwhline(win, 7, 34, ACS_HLINE, 5);
			wmove(win, y, x);
			write = 0;
		}
		if(count == 0 ) {
			deletewin :
			if(swin != NULL) {
				unpost_menu(gmenu);
				free_menu(gmenu);
				i = 0;
				while(gitem[i] != NULL) 
					free_item(gitem[i++]);
				wbkgd(swin, COLOR_PAIR(3));
				wclear(swin);
				delwin(swin);
				swin = NULL;
				mvwhline(win, 7, 2, ACS_HLINE, 30);
				mvwhline(win, 9, 2, ACS_HLINE, 30);
				mvwprintw(win, 8, location[0], "%s", menu_names[0]);
				form_driver(form, REQ_NEXT_FIELD);
			}
		}
	}
	end:
	unpost_form(form);
	free_form(form);
	for(i = 0; i < 7;i++) {
		free_field(field[i]);
	}
	wmove(win, 3, 1);
	wclrtobot(win);
	wmove(win, 1, 3);
	wrefresh(win);
}
/*function to create the invoice menu*/
void invoice(WINDOW *win) {
	MENU *menu3;
	ITEM **item;
	char *item_names[] = {"New Invoice", "View Previous Invoices"};
	int i = 0, size, x, y;
	size = size(item_names);
	
	init_pair(41, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(42, COLOR_RED, COLOR_GREEN);
	init_pair(43, COLOR_BLACK, COLOR_WHITE);
	
	item = (ITEM**)malloc( (size + 1) * sizeof(ITEM*));
	while(i < size) {
		item[i] = new_item(item_names[i], item_names[i]);
		i++;
	}
	item[i] = (ITEM*)NULL;
	menu3 = new_menu(item);
	menu_opts_off(menu3, O_SHOWDESC);
	set_menu_win(menu3, win);
	set_menu_sub(menu3, derwin(win, 1,  50, 1, (COLS - 48) / 2));
	set_menu_fore(menu3, COLOR_PAIR(42));
	set_menu_back(menu3, COLOR_PAIR(41));
	set_menu_format(menu3, 1, size);
	set_menu_mark(menu3, ">");
	post_menu(menu3);
	getmaxyx(win, y, x);
	mvwhline(win, 2, 1, ACS_HLINE, x - 2);
	mvwaddch(win, 2, x - 1, ACS_RTEE);
	mvwaddch(win, 2, 0, ACS_LTEE);
	wrefresh(win);
	
	while( (i = wgetch(win)) ) {
		switch(i) {
			case KEY_RIGHT:
				menu_driver(menu3, REQ_NEXT_ITEM);
				break;
			case KEY_LEFT:
				menu_driver(menu3, REQ_PREV_ITEM);
				break;
			case 27:
				goto end;
				break;
			case 10:
				if(current_item(menu3) == item[0]) {
					newinvoice(win);
				}
				else if(current_item(menu3) == item[1]) {
					previous_invoice(win);
				}
				box(win, 0, 0);
				break;
		}
		wrefresh(win);
	}
	end:
	unpost_menu(menu3);
	free_menu(menu3);
	free_item(item[0]);
	free_item(item[1]);
	free(item);
	wclear(win);
	box(win, 0, 0);
	wrefresh(win);
}
/*function to create the profile , edit them and storing in the file*/
void profile(WINDOW *win) {
	FIELD **field;
	FORM *form;
	int i = 0, x, y, size;
	char a[52], *field_names[] = {"Proprietor Name", "Store Name", "Address", "Reg. No", "Save" };
	size = size(field_names);
	FILE *fp;
	fp = fopen("profile.csv", "r+");
	if(fp == NULL)
		fp = fopen("profile.csv", "w+");
	init_pair(31, COLOR_BLACK, COLOR_WHITE);
	init_pair(32, COLOR_BLACK, COLOR_WHITE);
	init_pair(33, COLOR_RED, COLOR_GREEN);
	
	field = (FIELD**)malloc((size + 1) * sizeof(FIELD*));
	while(i < size - 1) {
		mvwprintw(win, 3 + 2 * i, 4, "%s : ", field_names[i]);
		field[i] = new_field(1, 50, 2 + 2 * i, 1, 0, 0);
		field_opts_off(field[i], O_AUTOSKIP);
		set_field_back(field[i], COLOR_PAIR(32));
		set_field_fore(field[i], COLOR_PAIR(32));
		i++;
	}
	field[i] = new_field(1, 6, 2 + 2 * i, 10, 0, 0);
	field_opts_off(field[i], O_AUTOSKIP | O_EDIT);
	set_field_back(field[i], COLOR_PAIR(33));
	set_field_fore(field[i], COLOR_PAIR(33));
	set_field_buffer(field[i], 0, " Save ");
	field[++i] = NULL;
	
	form = new_form(field);
	scale_form(form, &y, &x);
	
	set_form_win(form, win);
	set_form_sub(form, derwin(win, y, x, 1, 25));
	
	post_form(form);
	i = 0;
	while(fgets(a, 52, fp) != NULL) {
		set_field_buffer(field[i], 0, a);
		i++;
	}
	wrefresh(win);
	
	while( (i = wgetch(win)) ) {
		if(isprint(i) || (i == ' ') || (i == '\t') )
			form_driver(form, i);
		else {
			switch(i) {
				case KEY_DOWN:
					form_driver(form, REQ_NEXT_FIELD);
					form_driver(form, REQ_END_LINE);
					break;
				case KEY_UP:
					form_driver(form, REQ_PREV_FIELD);
					form_driver(form, REQ_END_LINE);
					break;
				case KEY_LEFT:
					form_driver(form, REQ_PREV_CHAR);
					break;
				case KEY_RIGHT:
					form_driver(form, REQ_NEXT_CHAR);
					break;
				case 27:
					goto end;
					break;
				case KEY_BACKSPACE: case 127:
					form_driver(form, REQ_DEL_PREV);
					break;
				case KEY_DC:
					form_driver(form, REQ_DEL_CHAR);
					break;
				case 10:
					if(current_field(form) == field[4]) {
						fseek(fp, 0, SEEK_SET);
						for(i = 0;i < 4;i++)
							fprintf(fp, "%-50s\n", field_buffer(field[i], 0));
						getmaxyx(win, y, x);
						mvwprintw(win, y - 2, 5, "Data Updated Successfully");
						wrefresh(win);
					}
					break;
			}
			
		}
	}
	end:
	fclose(fp);
	unpost_form(form);
	free_form(form);
	i = 0;
	while(i < size) {
		wmove(win, 3 + 2 * i, 1);
		wclrtoeol(win);
		free_field(field[i]);
		i++;
	}
	free(field);
	wclear(win);
	box(win, 0, 0);
	wrefresh(win);
	
}
/*Function to display items and provide search options using keys when inventory option is selected*/
void inventory(WINDOW *win) {
	ITEM **item;
	MENU *menu2;
	WINDOW *menu2win, *subwin1, *subwin2, **dpad;
	FIELD **field = (FIELD**)malloc(3 * sizeof(FIELD*));
	FORM *form;
	int i = 0, x, y, scroll = 0;
	char *options[] = {"Name", "Category", "Batch No", "Location", "Expiry Date", "Quantity"};
	char *item_names[] = {"Name", "Company", "Date", "Category", "Batch No.", "Location", "Expiry Date", "Quantity", "Price"};
	int size = size(options);
	int location[] = {7, 37, 58, 69, 81, 94, 107, 120, 130};
	char a[31];
	
	getmaxyx(win, y, x);
	init_pair(21, COLOR_BLACK, COLOR_WHITE);
	init_pair(22, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(23, COLOR_RED, COLOR_GREEN);
	
	menu2win = newwin(8, COLS - 4, 9, 2);
	box(menu2win, 0, 0);
	wbkgd(menu2win, COLOR_PAIR(22));
	mvwprintw(menu2win, 6, 1, "Sr No");
	while( i < size(location) ) {
		mvwprintw(menu2win, 6, location[i], "%s",item_names[i]);
		mvwaddch(menu2win, 6, location[i] - 1, ACS_VLINE);
		i++;
	}
	mvwhline(menu2win, 5, 1, ACS_HLINE, x - 2);
	mvwaddch(menu2win, 7 , 0, ACS_LTEE);
	mvwaddch(menu2win, 7, x - 1, ACS_RTEE);
	
	item = (ITEM**)malloc((size + 1) * sizeof(ITEM*));
	i = 0;
	while(i < size) {
		item[i] = new_item(options[i], "");
		i++;
	}
	item[i] = (ITEM*)NULL;
	menu2 = new_menu((ITEM**)item);
	keypad(menu2win, TRUE);
	menu_opts_off(menu2, O_SHOWDESC);
	
	subwin1 = derwin(menu2win, 1, COLS - 10, 2, 2);
	set_menu_win(menu2, menu2win);
	set_menu_sub(menu2, subwin1);
	set_menu_format(menu2, 1, size);
	set_menu_mark(menu2, ">");
	set_menu_fore(menu2, COLOR_PAIR(23));
	set_menu_back(menu2, COLOR_PAIR(22));
	
	post_menu(menu2);
	mvwprintw(menu2win, 1, 2, "Select Search Criteria ->"); 
	
	field[0] = new_field(1, 30, 0, 0, 0, 0);
	field[1] = NULL;
	
	set_field_back(field[0], COLOR_PAIR(21));
	set_field_fore(field[0], COLOR_PAIR(21));
	field_opts_off(field[0], O_AUTOSKIP);
	form = new_form(field);
	
	subwin2 = derwin(menu2win, 1, 60, 4, 2);
	set_form_win(form, menu2win);
	set_form_sub(form, subwin2);
	post_form(form);
	wrefresh(win);
	
	dpad = search_inventory(item_name(current_item(menu2)), "", location);
	getmaxyx(win, y, x);
	print_inventory(dpad, win, 0, y - 9);
	
	while( (i = wgetch(menu2win)) ) {
		switch(i) {
			case KEY_UP:
				getmaxyx(win, y, x);
				if(match > y - 9 && scroll > 0)
					scroll--;
				print_inventory(dpad, win, scroll, y - 9);
				break;
			case KEY_DOWN:
				getmaxyx(win, y, x);
				if(y - 9 + scroll < match)
					scroll++;
				print_inventory(dpad, win, scroll, y - 9);
				break;
			case KEY_RIGHT:
				menu_driver(menu2, REQ_RIGHT_ITEM);
				break;
			case KEY_LEFT:
				menu_driver(menu2, REQ_LEFT_ITEM);
				break;
			case 10://Enter key
				mvwprintw(menu2win, 3, 2, "Enter %s :- ", item_name(current_item(menu2)));
				a[0] = '\0';
				wmove(menu2win, 4, 2);
				while( (i = wgetch(menu2win)) ) {
					if(i == 27) {
						set_field_buffer(field[0], 0, "");
						a[0] = '\0';
						dpad = search_inventory(item_name(current_item(menu2)), "", location);
						getmaxyx(win, y, x);
						print_inventory(dpad, win, 0, y - 9);
						scroll = 0;
						break;
					}
					else if(i == KEY_LEFT) 
						form_driver(form, REQ_PREV_CHAR);
					else if(i == KEY_RIGHT) 
						form_driver(form, REQ_NEXT_CHAR);
					else if(i == KEY_UP) {
						getmaxyx(win, y, x);
						if(match > y - 9 && scroll > 0)
							scroll--;
						print_inventory(dpad, win, scroll, y - 9);
					}
					else if(i == KEY_DOWN) {
						getmaxyx(win, y, x);
						if(y - 9 + scroll < match)
							scroll++;
						print_inventory(dpad, win, scroll, y - 9);
					}
					else {
						getyx(menu2win, y, x);
						if( i == 127 || i == KEY_BACKSPACE ) {
							form_driver(form, REQ_DEL_PREV);
							swap_fore(x - 2, a);
						}
						else if(i == KEY_DC) {
							form_driver(form, REQ_DEL_CHAR);
							swap_fore(x - 1, a);
						}
						else if(isprint(i) || (i == ' ') || (i == '\t') ) {
							form_driver(form, i);
							swap_back(x - 2, a);
							a[x - 2] = i;
						}
						for(;match > 0;match--)
							delwin(dpad[match]); 
						dpad = search_inventory(item_name(current_item(menu2)), a, location);
						getmaxyx(win, y, x);
						if(match < y - 9) {
							wmove(win, 8 + match, 3);
							wclrtobot(win);
						}
						print_inventory(dpad, win, 0, y - 9);
						scroll = 0;
					}
					wrefresh(win);
				}
				break;
			case 27://Escape key
				goto end;
				break;
			
		}
		wmove(menu2win, 3, 1);
		wclrtoeol(menu2win);
		wrefresh(menu2win);
	}
	
	end:
	wborder(menu2win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	unpost_menu(menu2);
	free_menu(menu2);
	i = 0;
	while (i <= size) {
		wmove(menu2win, i, 1);
		wclrtoeol(menu2win);
		free_item(item[i]);
		i++;
	}
	unpost_form(form);
	free_form(form);
	free_field(field[0]);
	free_field(field[1]);
	wrefresh(menu2win);	
	delwin(menu2win);
	werase(win);
	wrefresh(win);	
}
/*function to create fields for adding new item , removing item and updating item when Add/Remove item option is selected*/
void newitem(WINDOW *formwin) {
	FIELD **field;
	FORM *form;
	WINDOW *swin = NULL;
	ITEM *item[3];
	MENU *menu;
	int i = 0, x, y, size, flag, count = 0, repeat, choice;
	char *field_names[] = {"Name *", "Company *", "Date", "Category *", "Batch No. *", "Location", "Expiry Date *", "Quantity *", "Price *"}, *arr, str[12];
	char *emptystr = "                              ";
	time_t t = time(0);
	struct tm *time = localtime( &t );
	sprintf(str, "%2d/%2d/%4d",time->tm_mday,time->tm_mon,1900 + time->tm_year);
	
	size = size(field_names);
	init_pair(11, COLOR_RED, COLOR_BLACK);
	init_pair(12, COLOR_BLACK, COLOR_WHITE);
	init_pair(13, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(16, COLOR_RED, COLOR_GREEN);
	
	item[0] = new_item("Add / Modify item", "");
	item[1] = new_item("Remove item", "");
	item[2] = (ITEM *)NULL;
	menu = new_menu(item);
	set_menu_win(menu, formwin);
	set_menu_sub(menu, derwin(formwin, 1, 40, 1, (COLS - 42) / 2));
	set_menu_format(menu, 1, 2);
	set_menu_fore(menu, COLOR_PAIR(16));
	set_menu_back(menu, COLOR_PAIR(13));
	post_menu(menu);
	wrefresh(formwin);
	field = (FIELD**)malloc((size + 2) * sizeof(FIELD*));
	
	menu:
	getmaxyx(formwin, y, x);
	mvwhline(formwin, 2, 1, ACS_HLINE, x - 2);
	mvwaddch(formwin, 2, x - 1, ACS_RTEE);
	mvwaddch(formwin, 2, 0, ACS_LTEE);
	
	while( (i = wgetch(formwin)) ) {
		switch(i) {
			case KEY_LEFT:
				menu_driver(menu, REQ_PREV_ITEM);
				break;
			case KEY_RIGHT:
				menu_driver(menu, REQ_NEXT_ITEM);
				break;
			case 10:
				field[size] = new_field(1, 8, 23, 33, 0, 0);
				field_opts_off(field[size], O_AUTOSKIP | O_EDIT);
				set_field_back(field[size], COLOR_PAIR(16));
				set_field_fore(field[size], COLOR_PAIR(16));
				if(current_item(menu) == item[0]) {
					set_field_buffer(field[size], 0, "   Add  ");
					choice = 1;
				}
				else {
					set_field_buffer(field[size], 0, " Remove ");
					choice = 2;
				}
				goto form;
				break;
			case 27:
				goto end;
				break;
		}
		wrefresh(formwin);
	}
	
	form:
	for(i = 0;i < 8;i += 2) {
		mvwprintw(formwin, 4 + 3 * i, 10, "%s : ", field_names[i]);
		field[i] = new_field(1, 30, 3 * i, 1, 0, 0);
		field_opts_off(field[i], O_AUTOSKIP);
		set_field_back(field[i], COLOR_PAIR(12));
		set_field_fore(field[i], COLOR_PAIR(12));
		
		field[i + 1] = new_field(1, 30, 2 * i, 55, 0, 0);
		field_opts_off(field[i + 1], O_AUTOSKIP);
		set_field_back(field[i + 1], COLOR_PAIR(12));
		set_field_fore(field[i + 1], COLOR_PAIR(12));
	}
	field[i] = new_field(1, 30, 2 + 2 * i, 55, 0, 0);
	field_opts_off(field[i], O_AUTOSKIP);
	set_field_back(field[i], COLOR_PAIR(12));
	set_field_fore(field[i], COLOR_PAIR(12));
	if(choice == 1)
		set_field_buffer(field[2], 0, str);
	field_opts_off(field[2], O_EDIT);
	field[size + 1] = NULL;
	
	form = new_form(field);
	scale_form(form, &y, &x);
	set_form_win(form, formwin);
	set_form_sub(form, derwin(formwin, y, x, 4, 25));
	post_form(form);
	for(i = 0;i < 8;i += 2)
		 mvwprintw(formwin, 4 + 2 * i, 65, "%s : ", field_names[i + 1]);
	mvwprintw(formwin, 6 + 2 * i, 65, "%s : ", field_names[i]);
	form_driver(form, REQ_FIRST_FIELD);
	wrefresh(formwin);
	
	while( (i = wgetch(formwin)) ) {
		wmove(formwin, 29, 1);
		wclrtoeol(formwin);
		if(isprint(i) || (i == ' ') || (i == '\t') ) {
			if(i == ' ')
				form_driver(form, REQ_NEXT_CHAR);
			else {
				form_driver(form, i);
				form_driver(form, REQ_END_LINE);
			}
			if(current_field(form) == field[0]) {
				count++;
				swin = medicine_name(formwin, swin, field[0], count, 5, 26);
			}
		}
		else {
			switch(i) {
				case KEY_DOWN:
					if(current_field(form) == field[0] && swin != NULL) {
						menu_driver(gmenu, REQ_DOWN_ITEM);
						wrefresh(swin);
					}
					else {
						form_driver(form, REQ_NEXT_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				case KEY_UP:
					if(current_field(form) == field[0] && swin != NULL) {
						menu_driver(gmenu, REQ_UP_ITEM);
						wrefresh(swin);
					}
					else {
						form_driver(form, REQ_PREV_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				case KEY_LEFT:
					form_driver(form, REQ_PREV_CHAR);
					break;
				case KEY_RIGHT:
					form_driver(form, REQ_NEXT_CHAR);
					break;
				case 27:
					if(current_field(form) == field[0] && swin != NULL) {
						set_field_buffer(field[0], 0, "");
						goto deletewin;
					}
					else {
						unpost_form(form);
						free_form(form);
						for(i = 0;i <= size;i++)
							free_field(field[i]);
						wmove(formwin, 2, 0);
						wclrtobot(formwin);
						box(formwin, 0, 0);
						wrefresh(formwin);
						goto menu;
					}
					break;
				case KEY_BACKSPACE: case 127:
					form_driver(form, REQ_DEL_PREV);
					if(current_field(form) == field[0] && count > 0) {
						count--;
						for(i = 1;i < 9;i++)
							set_field_buffer(field[i], 0, "");
						if(choice == 1)
							set_field_buffer(field[2], 0, str);
						if(count > 0)	
							swin = medicine_name(formwin, swin, field[0], count, 5, 26);
					}
					break;
				case KEY_DC:
					form_driver(form, REQ_DEL_CHAR);
					if(current_field(form) == field[0] && count > 0) {
						count--;
						for(i = 1;i < 9;i++)
							set_field_buffer(field[i], 0, "");
						if(choice == 1)
							set_field_buffer(field[2], 0, str);
						if(count > 0)	
							swin = medicine_name(formwin, swin, field[0], count, 5, 26);
					}
					break;
				case 10:
					if(current_field(form) == field[0]) {
						if(swin != NULL) {
							set_field_buffer(field[0], 0, item_name(current_item(gmenu)) );
							arr = field_buffer(field[0], 0);
							
							count = strlen(arr);
							while(arr[count - 1] == ' ')
								count--;
							repeat = set_buffer(field, current_item(gmenu));
							if(choice == 1)	
								set_field_buffer(field[2], 0, str);
							goto deletewin;
						}
					}
					else if( (current_field(form) == field[9]) && !strcmp(field_buffer(field[9], 0), "   Add  ") ) {
						flag = 0;
						i = 0;
						while(i < 9) {
							if(strcmp(field_buffer(field[i], 0), emptystr) == 0 && i != 2 && i != 5) {
								mvwprintw(formwin, 29, 8, "The field %s cannot be empty ", field_names[i]);
								flag = 1;
								break;
							}
							i++;
						}
						if(flag == 0) {
							i = newitem_process(form, field);
							if( i == 0 )
								mvwprintw(formwin, 29, 8, "Error Occured in saving ");
							else {
								mvwprintw(formwin, 29, 8, "Data saved Successfully");
								for(i = 0;i < size;i++)
									set_field_buffer(field[i], 0, "");
								count = 0;
								if(choice == 1)
									set_field_buffer(field[2], 0, str);
							}
						}
					}
					else if( (current_field(form) == field[9]) && !strcmp(field_buffer(field[9], 0), " Remove ") ) {
						remove_item(field, repeat);
						for(i = 0;i < 9;i++)
							set_field_buffer(field[i], 0, "");
					}
					else {
						form_driver(form, REQ_NEXT_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				default:
					getyx(formwin, y, x);
					mvwprintw(formwin, 24, 8, "Character not allowed");
					wmove(formwin, y, x);
					break;
			}
			if(count == 0 ) {
				deletewin :
				if(swin != NULL) {
					unpost_menu(gmenu);
					free_menu(gmenu);
					i = 0;
					while(gitem[i] != NULL) 
						free_item(gitem[i++]);
					wbkgd(swin, COLOR_PAIR(13));
					wclear(swin);
					delwin(swin);
					swin = NULL;
					form_driver(form, REQ_FIRST_FIELD);
					form_driver(form, REQ_END_LINE);
				}
			}
		}
	}
	end:
	for(i = 0;i < size;i++) {
		wmove(formwin, 3 + 2 * i, 1);
		wclrtoeol(formwin);
	}
	free(field);
	unpost_menu(menu);
	free_menu(menu);
	free_item(item[0]);
	free_item(item[1]);
	wclear(formwin);
	box(formwin, 0, 0);
	wrefresh(formwin);
}
/*function to print help for users*/
void help(WINDOW *win){
	mvwprintw(win, 2, 2, "All menus and fields can be handled only by keyboard");
	mvwprintw(win, 3, 2, "Use up, down, left and right arrow keys to navigate between menus and fields");
	mvwprintw(win, 4, 2, "Complete the profile before printing invoices");
	mvwprintw(win, 5, 2, "For more information visit ");
	wattron(win, A_UNDERLINE);
	wprintw(win, "github.com/aju20/mib_1.0.git");
	wattroff(win, A_UNDERLINE);
	
	while(wgetch(win) != 27)
		;
	wmove(win, 0, 0);
	wclrtobot(win);
	box(win, 0, 0);
	wrefresh(win);
}
int main() {
	WINDOW *menu1win, *mainwin;
	ITEM **item;
	MENU *menu1;
	int ch;
	char *title = "Medical Inventory and Billing 1.0";
	int i = 0, size;
	char *main_menu[] = {"Add/Remove item", "Inventory", "Invoice", "Profile", "Help", "Exit" };
	
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();
	init_pair(1, COLOR_RED, COLOR_GREEN);
	init_pair(3, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);
	init_pair(6, COLOR_BLACK, COLOR_YELLOW);
	
	attron(A_BOLD);
	mvprintw(1, (COLS - strlen(title) ) / 2, title);
	attroff(A_BOLD);
	bkgd(COLOR_PAIR(5));
	refresh();
	
	mainwin = newwin(LINES - 11, COLS - 4, 9, 2);/*create main big window in the program*/
	wbkgd(mainwin, COLOR_PAIR(3));
	box(mainwin, 0, 0);
	keypad(mainwin, TRUE);
	wrefresh(mainwin);
	
	menu1win = newwin(5, COLS - 4, 3, 2);/*create small window to display menu in the program*/
	keypad(menu1win, TRUE);
	wbkgd(menu1win, COLOR_PAIR(3));
	box(menu1win, 0, 0);
	
	size = size(main_menu);
	item = ( ITEM** )malloc((size + 1) * sizeof(ITEM*));
	while(i < size) {
		item[i] = new_item(main_menu[i], main_menu[i]);
		i++;
	}
	item[i] = (ITEM*)NULL;
	menu1 = new_menu((ITEM**)item);/*create the main menu in the program*/
	menu_opts_off(menu1, O_SHOWDESC);
	
	set_menu_win(menu1, menu1win);
	set_menu_sub(menu1, derwin(menu1win, 1, COLS - 10, 2, 4));
	set_menu_format(menu1, 1, size);
	set_menu_mark(menu1, "*"); 
	
	set_menu_fore(menu1, COLOR_PAIR(1));
	set_menu_back(menu1, COLOR_PAIR(3));
	
	post_menu(menu1);
	wrefresh(menu1win);
	
	while( (ch = wgetch(menu1win)) ) {
		switch(ch) {
			case KEY_RIGHT:
				menu_driver(menu1, REQ_RIGHT_ITEM);
				break;
			case KEY_LEFT:
				menu_driver(menu1, REQ_LEFT_ITEM);
				break;
			case 10:
				if( current_item(menu1) == item[5] ) {
					size = size(main_menu);
					unpost_menu(menu1);
					free_menu(menu1);
					i = 0;
					while(i < size + 1) {
						free_item(item[i]);
						i++;
					}
					endwin();
					return 0;
				}
				if(current_item(menu1) == item[4])
					help(mainwin);
				if(current_item(menu1) == item[0]) {
					newitem(mainwin);
				}
				if(current_item(menu1) == item[1]) {
					inventory(mainwin);
					box(mainwin, 0, 0);
					wrefresh(mainwin);
				}
				if(current_item(menu1) == item[3]) {
					profile(mainwin);
					box(mainwin, 0, 0);
					wrefresh(mainwin);
				}
				if(current_item(menu1) == item[2]) {
					invoice(mainwin);
					box(mainwin, 0, 0);
					wrefresh(mainwin);
				}
		}
		refresh();
	}
	
	unpost_menu(menu1);
	free_menu(menu1);
	i = 0;
	while(i < size)
		free_item(item[i++]);
	free(item);
	endwin();
	return 0;
}
