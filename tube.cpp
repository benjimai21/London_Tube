#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>

using namespace std;

#include "tube.h"

/* You are pre-supplied with the functions below. Add your own
	 function definitions to the end of this file. */

/* internal helper function which allocates a dynamic 2D array */
char** allocate_2D_array(int rows, int columns) {
	char** m = new char* [rows];
	assert(m);
	for(int r=0; r<rows; r++) {
		m[r] = new char[columns];
		assert(m[r]);
	}
	return m;
}

/* internal helper function which deallocates a dynamic 2D array */
void deallocate_2D_array(char** m, int rows) {
	for(int r=0; r<rows; r++)
		delete [] m[r];
	delete [] m;
}

/* internal helper function which gets the dimensions of a map */
bool get_map_dimensions(const char* filename, int& height, int& width) {
	char line[512];

	ifstream input(filename);

	height = width = 0;

	input.getline(line, 512);
	while(input) {
		if( (int) strlen(line) > width)
			width = strlen(line);
		height++;
		input.getline(line, 512);
	}

	if(height > 0)
		return true;
	return false;
}

/* pre-supplied function to load a tube map from a file*/
char** load_map(const char* filename, int& height, int& width) {

	bool success = get_map_dimensions(filename, height, width);

	if(!success) return NULL;

	char** m = allocate_2D_array(height, width);

	ifstream input(filename);

	char line[512];
	char space[] = " ";

	for(int r = 0; r<height; r++) {
		input.getline(line, 512);
		strcpy(m[r], line);
		while( (int) strlen(m[r]) < width ) {
			strcat(m[r], space);
		}
	}

	return m;
}

/* pre-supplied function to print the tube map */
void print_map(char** m, int height, int width) {
	cout << setw(2) << " " << " ";
	for(int c=0; c<width; c++) {
		if(c && (c % 10) == 0) {
			cout << c/10;
		} else {
			cout << " ";
		}
	}

	cout << endl;
	cout << setw(2) << " " << " ";

	for(int c=0; c<width; c++) cout << (c % 10);

	cout << endl;

	for(int r=0; r<height; r++) {
		cout << setw(2) << r << " ";
		for(int c=0; c<width; c++) cout << m[r][c];
		cout << endl;
	}
}

/* pre-supplied helper function to report the errors encountered in Question 3 */
const char* error_description(int code) {
  switch(code) {
  case ERROR_START_STATION_INVALID:
    return "Start station invalid";
  case ERROR_ROUTE_ENDPOINT_IS_NOT_STATION:
    return "Route endpoint is not a station";
  case ERROR_LINE_HOPPING_BETWEEN_STATIONS:
    return "Line hopping between stations not possible";
  case ERROR_BACKTRACKING_BETWEEN_STATIONS:
    return "Backtracking along line between stations not possible";
  case ERROR_INVALID_DIRECTION:
    return "Invalid direction";
  case ERROR_OFF_TRACK:
    return "Route goes off track";
  case ERROR_OUT_OF_BOUNDS:
    return "Route goes off map";
  }
  return "Unknown error";
}

/* presupplied helper function for converting string to direction enum */
Direction string_to_direction(const char* token) {
  const char* strings[] = {"N", "S", "W", "E", "NE", "NW", "SE", "SW"};
  for(int n=0; n<8; n++) {
    if(!strcmp(token, strings[n])) return (Direction) n;
  }
  return INVALID_DIRECTION;
}


/* Function to attribute coordinates to a symbol */
bool get_symbol_position(char** m, int rows, int columns, char symbol, int& r_x, int& c_x)
{
  if (!isalnum(symbol)) {
    r_x = -1;
    c_x = -1;
    return false;
  }
  
  for (r_x = 0 ; r_x < rows ; r_x++) {
    for (c_x = 0 ; c_x < columns ; c_x++) {

      if (isalnum(m[r_x][c_x])) {
	if (symbol == m[r_x][c_x]) {
	  return true;
	}
      }
    }
  }

  r_x = - 1;
  c_x = - 1;

  return false;
}                     


/* function to get the corresponding symbol of a station */
char get_symbol_for_station_or_line(const char a[])
{
  ifstream in_stream_station;
  ifstream in_stream_line;

  const int MAX_CHARACTERS = 50;
  char letter[MAX_CHARACTERS];

  in_stream_station.open("stations.txt");
  in_stream_line.open("lines.txt");

  for (int row = 0 ; !in_stream_station.fail() ; row++) { 
    in_stream_station.getline(letter, MAX_CHARACTERS);          
    for(int i = 0 ; a[i] == letter[i + 2] ; i++) {     /* look for the symbol corresponding to a station */  
      if (letter[i + 3] == '\0')
	return letter[0];                            
    }                                                  
  }                                                    /* if did not find the name in the station file look into the line file */

  for (int row = 0 ; !in_stream_line.fail() ; row++) {
    in_stream_line.getline(letter, MAX_CHARACTERS);
    for(int i = 0 ; a[i] == letter[i + 2] ; i++) {     /* look for the symbol corresponding to a line */   
      if (letter[i + 3] == '\0')
	return letter[0];
    }
  }                       

  in_stream_station.close();
  in_stream_line.close();
  return ' ';                                          /* if did not find the name in the line file return an empty value */ 

}

/* function that indicates an end station and the number of line changes to get that station given a start station and a route direction */
int validate_route(char** m, int rows, int columns, const char* station, char* route, char destination[]) {

  char start_location = get_symbol_for_station_or_line(station);
  if (!isalnum(start_location)) {
    return ERROR_START_STATION_INVALID;
  }

  int r, c, previous_r, previous_c;
  if (!get_symbol_position(m, rows, columns, start_location, r, c)) {
    return ERROR_START_STATION_INVALID;
  }

  previous_r = r;       /*previous_r and previous_c refer to the coordinates in the last iteration, r and c are the coordinates in the current iteration */   
  previous_c = c;
  
  char next_direction[3];
  int i = 0;
  int train_change = 0;

  while (route[i] != '\0') {
    int index_direction = 0;
    for (;route[i] != ',' &&  route[i] != '\0' ; index_direction++, i++) {

      if (index_direction == 2) {
	return ERROR_INVALID_DIRECTION;
      }
    
      next_direction[index_direction] = route[i];

    }
    next_direction[index_direction] = '\0';
    if(route[i] == ',') {
      i++;
    }

    Direction direction = string_to_direction(next_direction);
    
    int new_r, new_c;    
    switch (direction) { /* determines the new coordinates at each new direction */
    case N:
      new_r = r - 1;     
      new_c = c;
      break;
    case S:
      new_r = r + 1;      
      new_c = c;
      break;
    case W:
      new_r = r;
      new_c = c - 1;
      break;
    case E:
      new_r = r;
      new_c = c + 1;
      break;
    case NE:
      new_r = r - 1;
      new_c = c + 1;
      break;
    case NW:
      new_r = r - 1;
      new_c = c - 1;
      break;
    case SE:
      new_r = r + 1;
      new_c = c + 1;
      break;
    case SW:
      new_r = r + 1;
      new_c = c - 1;
      break;
    default:
      return ERROR_INVALID_DIRECTION;
    }
    
    if (new_r < 0 || new_r > rows || new_c < 0 || new_c > columns)
      return ERROR_OUT_OF_BOUNDS;

    if (m[new_r][new_c] == ' ')
      return ERROR_OFF_TRACK;

    if ((!isalnum(m[new_r][new_c]) && (!isalnum(m[r][c])) && m[new_r][new_c] != m[r][c]))
      return ERROR_LINE_HOPPING_BETWEEN_STATIONS;

    if ((!isalnum(m[r][c])) && new_r == previous_r && new_c == previous_c)
      return ERROR_BACKTRACKING_BETWEEN_STATIONS;

    if (isalnum(m[r][c]) && (!isalnum(m[previous_r][previous_c])) && (!isalnum(m[new_r][new_c])) && ((m[new_r][new_c] != m[previous_r][previous_c]) || ((new_r == previous_r) && (new_c == previous_c))))
      train_change++; 
    
  previous_r = r;
  previous_c = c;
  r = new_r;
  c = new_c;
  
  }
  
  if (!isalnum(m[r][c])) 
      return ERROR_ROUTE_ENDPOINT_IS_NOT_STATION;
            
get_final_station(m[r][c], destination);
     
      return train_change;
     
}
       
            
/* function that finds the name of the station corresponding to the end point symbol */
void get_final_station(char symbol, char dest[])
{
  ifstream in_stream_station;
  char letter[100];
  int i = 0;
  in_stream_station.open("stations.txt");

  while(!in_stream_station.eof()) {

    in_stream_station.getline(letter, 100);
    if (letter[0] == symbol) {
      for (int j = i + 2 ; letter[j] != '\0' ; j++, i++)
	dest[i] = letter[j];
    }
  }
  dest[i] = '\0';
  in_stream_station.close();
}
      
 
