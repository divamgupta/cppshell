/*


	Author - Divam Gupta

	Resuirements GCC 4.9. and C++ 11


	sudo apt-get install build-essential
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
	sudo apt-get update
	sudo apt-get install gcc-4.9 g++-4.9  cpp-4.9

	g++ -std=c++11 shell.cpp 

	g++-4.9 -std=c++11 shell.cpp

*/



#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstring>

#include <fcntl.h>



#include <stdlib.h>	 
#include <unistd.h>	 
#include <stdio.h>	 
#include <stdlib.h>	
#include <signal.h>	

#include <sys/types.h>
#include <sys/wait.h>


using namespace std;




void sigintHandler(int sig_num)
{
   
    signal(SIGINT, sigintHandler);
  
}



// some inbuilt functions in my bash

void history()
{
	string getcontent;
    ifstream openfile ( (string)P_tmpdir + "theHistoty.txt" );

    if(openfile.is_open())
    {
        while(! openfile.eof())
        {
             getline(openfile, getcontent);
            cout << getcontent << endl;
        }
    }


	exit(1);
}

void help()
{
	cout << " ****************	" << endl;
	cout << "Hey .. YO 	 	" << endl;
	cout << "Welcome to the my shell :P 	" << endl;
	cout << "Commands" << endl;
	cout << "1) cd dir" << endl;
	cout << "2) history" << endl;
	cout << "3) help" << endl;
	cout << "4) exit" << endl;
	cout << "5) multiple pipes   A | B | C" << endl;
	cout << "  	" << endl;
	cout << "  	" << endl;

	exit(1);
}



void logHistory( string s )
{
	ofstream outfile;
	outfile.open( (string)P_tmpdir + "theHistoty.txt", std::ios_base::app);
	outfile << s << endl; 

	

}

 char * const * splitString( string s){

   // http://stackoverflow.com/questions/5888022/split-string-by-single-spaces
 	 char str[290];
 	 strcpy( str , s.c_str());

	 int nSpaces = count( s.begin(), s.end(), ' ' );
	 char *args[ nSpaces + 2 ];

	 char * pch;
     pch = strtok (str," ");

     int i =0;

     while (pch != NULL)
	 {
	 		string tmp = (string)pch;
	 		args[i] = new char[tmp.length() + 1];
	 	   strcpy(args[i] , tmp.c_str() );
	       pch = strtok (NULL, " ");
	       i++;

	 }

	 args[ nSpaces + 1 ] = 0 ;

	 return args;
}


string fixifyCommand(string command)
{
	command =  regex_replace(command, regex("^ +| +$|( ) +"), "$1");
	return command;
	// http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
}





void runCommandRaw( string s , int fd0 , int fd1)
{
	s = fixifyCommand(s);

	char * const *  argsa =  splitString( s ) ;
	int nSpaces = count( s.begin(), s.end(), ' ' );
	char * args[nSpaces + 2];
	for(int i=0 ; i < nSpaces+2; i++)
		args[i] = argsa[i];
	char func[40];
		strcpy(func, args[0]);


	if(strcmp(func , "cd") == 0)
	{
		chdir( args[1] );
		return;
	}
	else if(strcmp(func , "exit") == 0)
	{
		exit(1);
		return;
	}


	int pid = fork();

	if(pid > 0)
	{
		wait(NULL);
		return;
	}

	if( pid == 0)
	{
		dup2(fd0, 0);
		dup2(fd1, 1);
		

		if(strcmp(func , "history") == 0)
		{
			history();
		}
		else if(strcmp(func , "help") == 0)
		{
			help();
		}
		else
		{
			execvp(func, args);
			if( s != "")
				cout << "Command  "<<  s <<" not found" << endl;


			exit(1);
		}
		
	}

}



void runCommandWithPipe( string s , int fd0 , int fd1)
{

	int firstPipePos = s.find("|");
	int nComamnds;

	string firstPart;  string restPart;

	if(firstPipePos!=std::string::npos)
    { 
    	nComamnds = 2;
        firstPart = s.substr ( 0 , firstPipePos ) ;
       	restPart =   s.substr ( firstPipePos +1, s.length() ) ;
    }
    else
    {
    	nComamnds = 1;
    	firstPart = s;
    }

    if(nComamnds == 1)
    {
    	runCommandRaw(firstPart , fd0 , fd1);
    }
    else
    {
    	int fd[2];
    	pipe(fd);

    	runCommandRaw(firstPart , fd0 , fd[1]);
    	close(fd[1]);
    	runCommandWithPipe( restPart ,  fd[0] ,  fd1);

    }
}


void runCommand( string s)
{

	s = fixifyCommand(s);

	char str[290];
 	strcpy( str , s.c_str());

 	char * pch;
    pch = strtok (str," ");

    vector<string> v;

    string outputFile = "";
    string inputFile = "";

	 while (pch != NULL)
	 {
	 		string tmp = (string)pch;
	 		v.push_back(tmp);
	 		 pch = strtok (NULL, " ");
	 }

	 for(int i=0 ; i < v.size() ; i++)
	 {
	 	if( v[i].compare(">") == 0 )
	 	{
	 		v[i] = "";
	 		outputFile = v[i+1];
	 		v[i+1] = "";

	 	}

	 	if( v[i].compare("<") == 0 )
	 	{
	 		v[i] = "";
	 		inputFile = v[i+1];
	 		v[i+1] = "";

	 	}

	 }

	 // fjoin string back form the vector an dexecute this commnd
	 s = "";
	 for(int i=0 ; i < v.size() ; i++)
	 	s += v[i] + " ";


	 int inputFD = 0;
	 int outputFD = 1;

	 if( outputFile.compare( "") != 0 )
	 {
	 	outputFD  = open(  outputFile.c_str() , O_CREAT|O_TRUNC|O_WRONLY, 0644);
	 }

	 if( inputFile.compare( "") != 0 )
	 {
	 	// cout << "INPUT " << inputFile << endl;
	 	inputFD  = open(  inputFile.c_str() , O_RDONLY);
	 }	 

	 runCommandWithPipe( s , inputFD , outputFD);


}

void openMyselfInNewWindow(char ** argv)
{
	int pid = fork();

	if(pid == 0)
	{
		// cout<< strcat( argv[0] , " d"  ) ;
		char * arg[] = { "gnome-terminal" , "-e" , strcat( argv[0] , " d"  ), 0};
		execvp("gnome-terminal" , arg);
		printf("YOU UDEOT\n");
	}

	if(pid > 0)
	{
		wait(NULL);
		exit(1);
	}
}

int main(int argc , char ** argv )
{

	
	if(argc == 1)
	{
		//runCommand("gnome-terminal -e " + (string)argv[0] + "\\ a");
		//exit(1);
		openMyselfInNewWindow( argv);
	}


	signal(SIGINT, sigintHandler);

	while(1)
	{
		string s;
		cout << getcwd(NULL , 0);
		cout << "$ ";
		

		if (getline( cin , s) == NULL )
			exit(1);

		if( s != "")
			logHistory( s );


		runCommand(s);
	}
	
}
