// --------------------------------------------Include files
#include<iostream>
#include<stdio.h>
#include<string>
#include<string.h>
#include<vector>
#include<stdlib.h>
#include<termios.h>
#include<unistd.h>
#include<ctype.h>
#include<limits.h>
#include<dirent.h>
#include<sys/stat.h>
#include<algorithm>
#include<iomanip>
#include<sys/ioctl.h>
#include<stack>
#include<pwd.h>
#include<grp.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fstream>
#include<fcntl.h>


// --------------------------------------------namespace
using namespace std;

// --------------------------------------------macros
#define loop(i,vec) for(auto i=vec.begin(); i!= vec.end(); i++)
#define rep(i,a,b) for(int i=a;i<b;i++)
#define printVector(x) loop(i,x){ cout<<*i<<"|"; }cout<<"\n";
#define all(vec) vec.begin(),vec.end()

// --------------------------------------------variables
struct termios original_config;
string home_path;
bool resize_detected = false;

// -------variables main
int x_cord=1;
int start_record_ind=0;

vector<string> curr_path;
vector<string> curr_path_content;
vector<vector<string>> curr_path_content_info;
vector<int> cpci_maxlen(7);
vector<bool> is_directory;

stack<vector<string>> forward_hitory;
stack<vector<string>> backward_hitory;

bool normal_mode = true;
bool make_NM_screen = true;
bool need_of_content_info_refresh=true;

bool quit_flag=false;

// --------------------------------------------Functions
void moveCursor(int x, int y) {
    cout<<"\x1b["<<x<<";"<<y<<"H";
}

void stringToCharArr(string input, char ** output){
    int in_str_len = input.length();

    *output = new char[in_str_len+1];
    strcpy(*output, input.c_str());
}

void vectorToString(vector<string> & input, string & output){
    loop(i,input){
        output.push_back('/');
        output.append(*i);
    }
    // output.push_back('/');
}

void stringToVector(string & input, vector<string> & output){
    string temp;
    output.clear();

    int in_str_len = input.length();
    
    rep(i,1,in_str_len){
        char ith_char=input[i];

        if(ith_char == '/'){
            output.push_back(temp);
            temp.clear();
        }else{
            temp.push_back(ith_char);
        }
    }

    if(input[in_str_len-1] != '/'){
        output.push_back(temp);
    }
}

void diableNonCanonicalMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_config);
}

void enableNonCanonicalMode(){
    tcgetattr(STDIN_FILENO, &original_config);
    atexit(diableNonCanonicalMode);

    struct termios new_config = original_config;
    new_config.c_lflag = ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_config);
}

void getStartPath(string & output){
    char path[4096];
    getcwd(path, sizeof(path));

    output = path;
    // output="/home/nisarg/assignments/aos/10-09-2020/dir_root";
}

void getCurrPathContent(){
    // input - current path
    // output - current path content (Sorted)

    string path_str;
    vectorToString(curr_path,path_str);

    char * path_char_ptr;
    stringToCharArr(path_str, &path_char_ptr);

    struct dirent *dir_entry_ptr;
    DIR *dir_ptr = opendir(path_char_ptr);

    while ((dir_entry_ptr = readdir(dir_ptr)))
    {
        curr_path_content.push_back(dir_entry_ptr->d_name);
    }    
    closedir(dir_ptr);

    sort(all(curr_path_content));
}

void getCurrPathContentInfo(){
    // input1 - current path
    // input2 - current path content (Sorted)
    
    // output1 - current path content info
    // output2 - current path content info maximum length

    
    const int NAME = 0;
    const int SIZE = 1;
    const int USR_OWNR = 2;
    const int GRP_OWNR = 3;

    
    string path_str;
    vectorToString(curr_path,path_str);

    loop(i,curr_path_content){

        string content_path_str = path_str + "/" + (*i);
        char * content_path_char;

        stringToCharArr(content_path_str, &content_path_char);

        struct stat file_info;
        stat(content_path_char, &file_info);

        vector<string> curr_content_info;
        int max_len;

        // Adding elements to --> curr_content_info

        // --------------------------------------------------------NAME
        curr_content_info.push_back(*i);

        max_len = (*i).length();
        if(max_len > cpci_maxlen[NAME]){ 
            cpci_maxlen[NAME] = max_len;
        }

        // --------------------------------------------------------SIZE
        string size_str = to_string(file_info.st_size);
        curr_content_info.push_back(size_str);

        max_len = (size_str).length();
        if(max_len > cpci_maxlen[SIZE]){ 
            cpci_maxlen[SIZE] = max_len;
        }

        // --------------------------------------------------------OWNERSHIP
        // -----user
        struct passwd *pw = getpwuid(file_info.st_uid);

        string user_str = pw->pw_name;
        curr_content_info.push_back(user_str);

        max_len = (user_str).length();
        if(max_len > cpci_maxlen[USR_OWNR]){ 
            cpci_maxlen[USR_OWNR] = max_len;
        }

        // -----group
        struct group *gr = getgrgid(file_info.st_gid);

        string grp_str = gr->gr_name;
        curr_content_info.push_back(grp_str);

        max_len = (grp_str).length();
        if(max_len > cpci_maxlen[GRP_OWNR]){ 
            cpci_maxlen[GRP_OWNR] = max_len;
        }

        // ------TYPE
        is_directory.push_back(S_ISDIR(file_info.st_mode));

        // --------------------------------------------------------PERMISSION
        string permission_str;

        S_ISDIR(file_info.st_mode) ? permission_str.push_back('d') : permission_str.push_back('-') ;

        file_info.st_mode & S_IRUSR ? permission_str.push_back('r') : permission_str.push_back('-') ;
        file_info.st_mode & S_IWUSR ? permission_str.push_back('w') : permission_str.push_back('-') ;
        file_info.st_mode & S_IXUSR ? permission_str.push_back('x') : permission_str.push_back('-') ;

        file_info.st_mode & S_IRGRP ? permission_str.push_back('r') : permission_str.push_back('-') ;
        file_info.st_mode & S_IWGRP ? permission_str.push_back('w') : permission_str.push_back('-') ;
        file_info.st_mode & S_IXGRP ? permission_str.push_back('x') : permission_str.push_back('-') ;

        file_info.st_mode & S_IROTH ? permission_str.push_back('r') : permission_str.push_back('-') ;
        file_info.st_mode & S_IWOTH ? permission_str.push_back('w') : permission_str.push_back('-') ;
        file_info.st_mode & S_IXOTH ? permission_str.push_back('x') : permission_str.push_back('-') ;

        curr_content_info.push_back(permission_str);

        // --------------------------------------------------------LAST_MODIFIED
        struct tm last_modified_time_info= *(gmtime(&file_info.st_mtime));
        
        // ------DATE
        string last_modified_date_info_str;
                
        last_modified_date_info_str.append(to_string(last_modified_time_info.tm_mday));
        last_modified_date_info_str.push_back('-');
        last_modified_date_info_str.append(to_string(last_modified_time_info.tm_mon));
        last_modified_date_info_str.push_back('-');
        last_modified_date_info_str.append(to_string(last_modified_time_info.tm_year + 1900));
        
        curr_content_info.push_back(last_modified_date_info_str);

        // ------TIME
        string last_modified_time_info_str;

        last_modified_time_info_str.append(to_string(last_modified_time_info.tm_hour));
        last_modified_time_info_str.push_back(':');
        last_modified_time_info_str.append(to_string(last_modified_time_info.tm_min));
        last_modified_time_info_str.push_back(':');
        last_modified_time_info_str.append(to_string(last_modified_time_info.tm_sec));
        
        curr_content_info.push_back(last_modified_time_info_str);

        // ------------- Adding current content info
        curr_path_content_info.emplace_back(curr_content_info);
    }
}

void printCurrPathContentInfo(){
    struct winsize curr_win_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &curr_win_size);
    int max_record_count = curr_win_size.ws_row;
    max_record_count--;

    // normal data printing
    int total_record=curr_path_content_info.size();
    int loop_end= max_record_count+start_record_ind-1;

    if(loop_end > total_record){
        loop_end=total_record-1;
    }

    rep(i,start_record_ind,loop_end+1){

        int ind=0;
        loop(j,curr_path_content_info[i]){
            cout<<setw(cpci_maxlen[ind])<<left<<*j<<" ";

            ind++;
        }cout<<"\n";
    }

    // status bar printing
    moveCursor(max_record_count+1,1);
    string curr_path_str;

    vectorToString(curr_path,curr_path_str);
    cout<<"Status-bar : "<<curr_path_str<<" ";
}

void normalModePreSteps(){
    
    cpci_maxlen[0]=INT_MIN; //NAME
    cpci_maxlen[1]=INT_MIN; //SIZE
    cpci_maxlen[2]=INT_MIN; //OWN_USR
    cpci_maxlen[3]=INT_MIN; //OWN_GRP

    cpci_maxlen[4]=10; //PERMISSIONS
    cpci_maxlen[5]=10; //LAST_MODIFIED_DATE
    cpci_maxlen[6]=8; //LAST_MODIFIED_TIME

    curr_path_content.clear();
    curr_path_content_info.clear();    
    is_directory.clear();

    start_record_ind=0;
    x_cord=1;
}

void makeNormalModeScreen(){  

    cout<<"\x1b[2J";
    cout<<"\x1b[H";


    // is there need of get content again
    if(need_of_content_info_refresh){
        normalModePreSteps();
        getCurrPathContent();
        getCurrPathContentInfo();
    }

    // printVector(cpci_maxlen);
    printCurrPathContentInfo();
    
    // to send back cursor to first line
    moveCursor(x_cord,1);
}

void makeCmdModeScreen(){

    rep(i,0,150)
        cout<<"\n";
    cout<<"\x1b[999;1H";

    // string curr_path_str;
    // vectorToString(curr_path, curr_path_str);

    // cout<<"\n"<<curr_path_str<<" >>>";
}

void cmdModePreWork(){
    // work any needed
}

void keyPressForNormalMode(){

    // k               107 k                1 0
    // l               108 l                1 0
    // up              27, 91 [, 65 A       1/0 0
    // down            27, 91 [, 66 B       1/0 0
    // enter           13                   1 1
    // left            27, 91 [, 68 D       1 1
    // right           27, 91 [, 67 C       1 1
    // backspace       127                  1 1 
    // h               104 h                1 1 

    // ---------------------------------------------- NEEDFUL VARIABLES
    int total_record = curr_path_content.size();

    struct winsize curr_win_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &curr_win_size);
    int max_record_count = curr_win_size.ws_row;
    max_record_count--;

    string next_path;

    make_NM_screen=true;
    need_of_content_info_refresh=true;

    char input_char;
    cin.get(input_char);

    if(input_char == '\x1b'){

        char esc_seq[2];
        cin.get(esc_seq[0]);
        cin.get(esc_seq[1]);

        switch (esc_seq[1])
        {
            case 'A':   // UP                
                if(x_cord>1){
                    cout<<"\x1b[A";
                    make_NM_screen=false;
                    x_cord--;
                }else{
                    if(start_record_ind == 0 ){
                        make_NM_screen=false;
                    }else{
                        start_record_ind--;
                    }
                }                

                need_of_content_info_refresh=false;
                break;
            
            case 'B':   // DOWN

                if(x_cord<max_record_count && x_cord+start_record_ind<total_record){
                    cout<<"\x1b[B";
                    make_NM_screen=false;
                    x_cord++;
                }else if(x_cord+start_record_ind==total_record){
                    make_NM_screen=false;
                }else{
                    if(start_record_ind + max_record_count == total_record ){
                        make_NM_screen=false;
                    }else{
                        start_record_ind++;
                    }
                }        

                need_of_content_info_refresh=false;
                break;
                
            case 'C':   // RIGHT
                if(!forward_hitory.empty()){
                    backward_hitory.push(curr_path);
                    curr_path=forward_hitory.top();
                    forward_hitory.pop();

                    x_cord=1;
                    start_record_ind=0;
                }else{
                    make_NM_screen=false;
                    need_of_content_info_refresh=false;
                }

                break;

            case 'D':   // LEFT
                if(!backward_hitory.empty()){
                    forward_hitory.push(curr_path);
                    curr_path=backward_hitory.top();
                    backward_hitory.pop();

                    x_cord=1;
                    start_record_ind=0;
                }else{
                    make_NM_screen=false;
                    need_of_content_info_refresh=false;
                }

                break;

            default:
                make_NM_screen=false;
                need_of_content_info_refresh=false;
                break;
        }

    }else{
        switch (input_char)
        {
            case 'q':
                quit_flag=true;
                break;
            
            case 13:    // enter
                next_path=curr_path_content[x_cord+start_record_ind-1];

                if(next_path[0] == '.'){
                    int next_path_len = next_path.length();

                    if(next_path_len ==1){
                        x_cord=1;
                        start_record_ind=0;
                    }else if(next_path_len == 2 && next_path[1] == '.'){
                        backward_hitory.push(curr_path);

                        curr_path.pop_back();
                        x_cord=1;
                        start_record_ind=0;
                    }else{
                        if(is_directory[x_cord+start_record_ind-1]){

                            backward_hitory.push(curr_path);

                            curr_path.push_back(next_path);
                            x_cord=1;
                            start_record_ind=0;
                        }else{
                            pid_t pid=fork();
                            
                            string file_path;
                            vectorToString(curr_path,file_path);

                            file_path = file_path+"/"+next_path;

                            if(pid==0){
                            execl("/usr/bin/vi","vi",file_path,NULL);
                            exit(0);
                            }
                            wait(NULL);
                        }
                    } 
                }else{
                    if(is_directory[x_cord+start_record_ind-1]){

                        backward_hitory.push(curr_path);

                        curr_path.push_back(next_path);
                        x_cord=1;
                        start_record_ind=0;
                    }else{
                        pid_t pid=fork();
                        
                        string file_path;
                        vectorToString(curr_path,file_path);

                        file_path = file_path+"/"+next_path;

                        char * file_path_char_ptr;
                        stringToCharArr(file_path, &file_path_char_ptr);

                        if(pid==0){
                        execl("/usr/bin/vi","vi",file_path_char_ptr,NULL);
                        exit(0);
                        }
                        wait(NULL);
                    }
                }

                while(!forward_hitory.empty()){
                    forward_hitory.pop();
                }

                break;
            
            case 127: // backspace

                backward_hitory.push(curr_path);

                curr_path.pop_back();
                x_cord=1;
                start_record_ind=0;

                while(!forward_hitory.empty()){
                    forward_hitory.pop();
                }

                break;

            case 'k':
                start_record_ind -= max_record_count;

                if(start_record_ind< 0){
                    start_record_ind=0;
                }
                x_cord=1;
                
                need_of_content_info_refresh=false;
                break;

            case 'l':
                if(start_record_ind+max_record_count < total_record ){
                    start_record_ind += max_record_count;
                    x_cord=1;
                }                
                
                need_of_content_info_refresh=false;
                break;

            case 'h':
                backward_hitory.push(curr_path);

                stringToVector(home_path,curr_path);     
                x_cord=1;
                start_record_ind=0;

                while(!forward_hitory.empty()){
                    forward_hitory.pop();
                }

                break;            

            case ':':
                normal_mode=false;

                makeCmdModeScreen();
                cmdModePreWork();

                break;

            default:
                make_NM_screen=false;
                need_of_content_info_refresh=false;
                break;
        }
    }
}

// ---------------------------------------------------------------CMD mode

__mode_t getPermissionNumber(string path_str){

    // mode_t == unsigned int

    char * content_path_char;
    stringToCharArr(path_str, &content_path_char);

    struct stat file_info;
    stat(content_path_char, &file_info);

    __mode_t permission=file_info.st_mode;
    return permission;
}

bool isDirectoryCheck(string path){
    char * path_char_ptr;
    stringToCharArr(path, &path_char_ptr);

    struct stat file_info;
    stat(path_char_ptr, &file_info);

    return S_ISDIR(file_info.st_mode);
}

bool checkValidityOfPath(string path){

    DIR *dir_pointer;

    char * path_char_ptr;
    stringToCharArr(path, &path_char_ptr);

    dir_pointer = opendir(path_char_ptr);

    if(dir_pointer == NULL){
        return false;
    }        
    return true;
}

void getCommandVector(vector<string> & command){
    string output_command_str;

    // output command string making
    char input_char;
    while(cin.get(input_char) && input_char != 13){

        if(input_char == 27){

            command.push_back("0");
            return;

            // cin.get(input_char);
            // if(input_char == 13){
            //     output_command.push_back("0");
            //     return;
            // }else{
            //     output_command_str.push_back(input_char);
            // }
        }

        if(input_char == 127){
            if(output_command_str.length()>0){



                cout<<"\x1b[D"<<" "<<"\x1b[D";
                output_command_str.pop_back();
            }
        }else{
            cout<<input_char;
            output_command_str.push_back(input_char);
        }
    }
    
    // command distribution
    string str_word;
    rep(i,0,output_command_str.length()){
        input_char = output_command_str[i];

        if(input_char==' '){
            if(str_word.length() > 0){
                command.push_back(str_word);
            }
            str_word = "";
        }else{
            str_word.push_back(input_char);
        }
    }  
    if(str_word.length() > 0){
        command.push_back(str_word);
    }


    // Manupulate command
    if(command.size() > 0){
        string command_type =command[0];
        
        if(!command_type.compare("copy")){
            command[0] = "1";
        
        }else if(!command_type.compare("move")){
            command[0] = "2";
        
        }else if(!command_type.compare("rename")){
            command[0] = "3";
        
        }else if(!command_type.compare("create_file")){
            command[0] = "4";
        
        }else if(!command_type.compare("create_dir")){
            command[0] = "5";
        
        }else if(!command_type.compare("delete_file")){
            command[0] = "6";
        
        }else if(!command_type.compare("delete_dir")){
            command[0] = "7";
        
        }else if(!command_type.compare("goto")){
            command[0] = "8";
        
        }else if(!command_type.compare("search")){
            command[0] = "9";
        
        }else if(!command_type.compare("q")){
            command[0] = "q";
        
        }else{        
            command.push_back(command[0]);
            command[0] = "-";
        } 
    }else{
        command.push_back("*");
    }
}

bool renameFile(string path_oldname_str, string path_newname_str){
    char * path_oldname_char_ptr;
    char * path_newname_char_ptr;

    stringToCharArr(path_oldname_str,&path_oldname_char_ptr);
    stringToCharArr(path_newname_str,&path_newname_char_ptr);

    int success = rename( path_oldname_char_ptr , path_newname_char_ptr );
    if ( success == 0 )
        return true;
    return false;
}

bool removeFile(string path_str){
    char * path_char_ptr;

    stringToCharArr(path_str,&path_char_ptr);

    int success = remove( path_char_ptr );
    if ( success == 0 )
        return true;
    return false;
}

bool createDir(string path_str, __mode_t permissions = 16877){
    char * path_char_ptr;
    stringToCharArr(path_str,&path_char_ptr);

    int success = mkdir( path_char_ptr , permissions );

    if ( success == 0 )
        return true;
    return false;
}

bool createFile(string path_str, __mode_t permissions = 33188){
    char * path_char_ptr;
    stringToCharArr(path_str,&path_char_ptr);

    int success = creat(path_char_ptr, permissions);;
    if ( success == 0 )
        return false;
    return true;
}

bool removeDir(string path_str){
    vector<string> path_content;

    char * path_char_ptr;
    stringToCharArr(path_str, &path_char_ptr);

    struct dirent *dir_entry_ptr;
    DIR *dir_ptr = opendir(path_char_ptr);

    while ((dir_entry_ptr = readdir(dir_ptr)))
    {
        path_content.push_back(dir_entry_ptr->d_name);
    }    
    closedir(dir_ptr);

    sort(all(path_content));

    string content_name_str;
    for(auto i=path_content.begin()+2; i!= path_content.end(); i++){
        content_name_str = path_str+"/"+(*i);
        
        if(isDirectoryCheck(content_name_str)){
            removeDir(content_name_str);
        }else{
            removeFile(content_name_str);
        }
    }

    stringToCharArr(path_str,&path_char_ptr);

    int success = remove( path_char_ptr );
    if ( success == 0 )
        return true;
    return false;
}

void copyData(string source_path_str, string dest_path_str){
    
    ifstream source_ptr(source_path_str, ios::binary);
    ofstream dest_ptr(dest_path_str, ios::binary);

    dest_ptr << source_ptr.rdbuf();

    source_ptr.close();
    dest_ptr.close();
}

void copyFile(string file_name, string source_path_str, string dest_path_str){
    source_path_str = source_path_str + "/" + file_name;
    dest_path_str = dest_path_str + "/" + file_name;

    __mode_t permissions = getPermissionNumber(source_path_str);

    bool return_val = createFile(dest_path_str, permissions);
    copyData(source_path_str, dest_path_str);
}

bool copyDir(string dir_name, string source_path_str, string dest_path_str){ 
    string source_path_dir_str = source_path_str + "/" + dir_name;
    string desti_path_dir_str = dest_path_str + "/" + dir_name;

    __mode_t permissions = getPermissionNumber(source_path_dir_str);
    createDir(desti_path_dir_str, permissions);

    vector<string> path_content;

    char * path_char_ptr;
    stringToCharArr(source_path_dir_str, &path_char_ptr);

    struct dirent *dir_entry_ptr;
    DIR *dir_ptr = opendir(path_char_ptr);

    while ((dir_entry_ptr = readdir(dir_ptr)))
    {
        path_content.push_back(dir_entry_ptr->d_name);
    }    
    closedir(dir_ptr);

    sort(all(path_content));

    string new_path_str;

    for(auto i=path_content.begin()+2; i!= path_content.end(); i++){
        new_path_str = source_path_dir_str + "/" + (*i);
        
        if(isDirectoryCheck(new_path_str)){
            copyDir((*i), source_path_dir_str, desti_path_dir_str);
        }else{
            copyFile((*i), source_path_dir_str, desti_path_dir_str);
        }
    }
}

void getAbsolutePath(string & relative_path, string & absolute_path){
    
    if(relative_path[0] == '~'){
        absolute_path = home_path + relative_path.substr(1);
    }else if(relative_path[0] == '/'){
        absolute_path = relative_path;
    }else{
        vectorToString(curr_path, absolute_path);
        absolute_path = absolute_path + "/" + relative_path;
    }

    // cout<<relative_path<<endl;
    // cout<<absolute_path<<endl;
}

bool searchInDir(string dir_path, string search_file_name){
    vector<string> path_content;

    char * path_char_ptr;
    stringToCharArr(dir_path, &path_char_ptr);

    struct dirent *dir_entry_ptr;
    DIR *dir_ptr = opendir(path_char_ptr);

    while ((dir_entry_ptr = readdir(dir_ptr)))
    {
        path_content.push_back(dir_entry_ptr->d_name);
    }    
    closedir(dir_ptr);

    loop(i,path_content){
        if((*i).compare(search_file_name) == 0){
            return true;  
        }
    }

    loop(i,path_content){
        if((*i).compare(search_file_name) == 0){
            return true;  
        }
    }

    string new_path_str;
    bool check_in_dir;

    loop(i,path_content){
        if((*i).compare(".") == 0 || (*i).compare("..") == 0){
            continue;  
        }else{
            new_path_str = dir_path + "/" + (*i);

            if(isDirectoryCheck(new_path_str)){
                check_in_dir = searchInDir(new_path_str, search_file_name);

                if(check_in_dir){
                    return true;
                }
            }
        }
    }

    return false;
}

void keyPressForCmdMode(){

    const char COPY         = '1';
    const char MOVE         = '2';
    const char RENAME       = '3';
    const char CREATE_FILE  = '4';
    const char CREATE_DIR   = '5';
    const char DELETE_FILE  = '6';
    const char DELETE_DIR   = '7';
    const char GOTO         = '8';
    const char SEARCH       = '9';
    const char ESC          = '0';
    const char QUIT         = 'q';
    const char BLANK        = '*';
    const char WRONG        = '-';
    
    string curr_path_str;
    vectorToString(curr_path, curr_path_str);

    cout<<"\n"<<curr_path_str<<" >>>";

    vector<string> command;
    getCommandVector(command);
    // printVector(command);

    // -----------------------------------Variables used in switch
    string file_path;
    string dir_path;
    string new_path_str;
    string dest_path_str;

    string search_file_name;

    string oldname_with_path;
    string newname_with_path;

    bool is_valid_path;    
    bool is_succesful_operation;

    string source_path_str;
    vectorToString(curr_path,source_path_str);

    // -----------------------------------switch
    switch(command[0][0]){
        case COPY:
            getAbsolutePath(command[command.size()-1], dest_path_str);            

            for(auto i=command.begin()+1; i!= command.end()-1; i++){

                // source_path_str is current path
                new_path_str = source_path_str + "/" + (*i);

                if(isDirectoryCheck(new_path_str)){
                    copyDir((*i), source_path_str, dest_path_str);
                }else{
                    copyFile((*i), source_path_str, dest_path_str);
                }
            }            
            cout<<"\n\nCopy Sucessful\n";

            break;
        
        case MOVE:
            getAbsolutePath(command[command.size()-1], dest_path_str);            

            for(auto i=command.begin()+1; i!= command.end()-1; i++){

                // source_path_str is current path
                new_path_str = source_path_str + "/" + (*i);

                if(isDirectoryCheck(new_path_str)){
                    copyDir((*i), source_path_str, dest_path_str);
                    removeDir(new_path_str);
                }else{
                    copyFile((*i), source_path_str, dest_path_str);
                    removeFile(new_path_str);
                }
            }            
            cout<<"\n\nMove Sucessful\n";

            break;
        
        case RENAME:

            getAbsolutePath(command[1], oldname_with_path);
            getAbsolutePath(command[2], newname_with_path);

            is_succesful_operation = renameFile(oldname_with_path,newname_with_path);
            if(is_succesful_operation){
                cout<<"\n\n'"<<command[1]<<"' renamed to '"<<command[2]<<"'\n";
            }else{
                cout<<"\n\nInput is wrong\n";
            }

            break;
        
        case CREATE_FILE:
            new_path_str = command[2] + '/' + command[1];
            getAbsolutePath(new_path_str, file_path);

            is_succesful_operation = createFile(file_path);
            if(is_succesful_operation){
                cout<<"\n\n'"<<command[1]<<"' - created\n";
            }else{
                cout<<"\n\nInput is wrong\n";
            }
            break;
        
        case CREATE_DIR:
            new_path_str = command[2] + '/' + command[1];
            getAbsolutePath(new_path_str, dir_path);

            cout<<dir_path<<endl;

            is_succesful_operation = createDir(dir_path);
            if(is_succesful_operation){
                cout<<"\n\n'"<<command[1]<<"' - created\n";
            }else{
                cout<<"\n\nInput is wrong\n";
            }

            break;
        
        case DELETE_FILE:

            getAbsolutePath(command[1], file_path);

            is_succesful_operation = removeFile(file_path);
            if(is_succesful_operation){
                cout<<"\n\n'"<<command[1]<<"' - has removed\n";
            }else{
                cout<<"\n\nInput is wrong\n";
            }
            break;
        
        case DELETE_DIR:

            getAbsolutePath(command[1], dir_path);

            is_succesful_operation = removeDir(dir_path);
            if(is_succesful_operation){
                cout<<"\n\n'"<<command[1]<<"' - has removed\n";
            }else{
                cout<<"\n\nInput is wrong\n";
            }
            break;
        
        case GOTO:
        
            backward_hitory.push(curr_path);

            getAbsolutePath(command[1], new_path_str);
            is_valid_path = checkValidityOfPath(new_path_str);

            if(is_valid_path){
                stringToVector(new_path_str,curr_path);
            }else{
                cout<<"\n\n'"<<command[1]<<"' - path is invalid\n";
            }     
            break;
        
        case SEARCH:
            search_file_name = command[1];          

            // source_path_str is current path
            is_succesful_operation = searchInDir(source_path_str, search_file_name);
            if(is_succesful_operation){
                cout<<"\n\nTrue\n";
            }else{
                cout<<"\n\nFalse\n";
            }
            break;
        
        case ESC:
            need_of_content_info_refresh=true;
            normal_mode=true;

            while(!forward_hitory.empty()){
                forward_hitory.pop();
            }
            
            break;
        
        case QUIT:
            quit_flag=true;
            break;
        
        case BLANK:
            break;
        
        case WRONG:
            cout<<"\n\n'"<<command[command.size()-1]<<"' - invalid command\n";
            break;        
    }
}

void checkForResize(int sig){

    int t_x_cord = x_cord;

    sleep(1);

    struct winsize curr_win_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &curr_win_size);
    int max_record_count = curr_win_size.ws_row;
    max_record_count--;

    if(normal_mode){

        cout<<"\x1b[2J";
        cout<<"\x1b[H";

        printCurrPathContentInfo();

        if(t_x_cord > max_record_count){
            x_cord = 1;
            cout<<"\x1b[H";
        }else{
            moveCursor(x_cord,1);
        }
    }
}

// --------------------------------------------main()
int main(){

    // --------------------------------------------Start Logic
    enableNonCanonicalMode();

    getStartPath(home_path);
    stringToVector(home_path,curr_path);

    signal(SIGWINCH, checkForResize);

    while(1){

        if(quit_flag){

            cout<<"start_record_ind: "<<start_record_ind<<"\n";
            cout<<"x_cord: "<<x_cord<<"\n";
            break;
        }

        if(normal_mode){
            if(make_NM_screen){
                makeNormalModeScreen();
            }

            keyPressForNormalMode();

        }else{
            keyPressForCmdMode();
        }
    }

    // temp code
    rep(i,0,150)
        cout<<"\n";
    cout<<"\x1b[H";
    cout<<"\n";   

    return 0;
}