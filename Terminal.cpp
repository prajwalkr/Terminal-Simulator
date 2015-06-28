// Author: K R Prajwal

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
using namespace std;
# define slash '/'
vector <string> valid_commands;
void init_valid_commands()
{
	valid_commands.push_back("pwd");
	valid_commands.push_back("cd");
	valid_commands.push_back("ls");
	valid_commands.push_back("touch");
	valid_commands.push_back("mkdir");
	valid_commands.push_back("rm");
	valid_commands.push_back("cat");
	valid_commands.push_back("chmod");
}
class node
{
public:
	friend class tree;
	string name;
	int inode;
	vector<node*> sub_dir_files;
	bool type;					// 0 = dir; 1 = file
	string content;
	string rwx[3];
	bool has_default_dir_permission;
/*#	Permission					rwx
7	read, write and execute		111
6	read and write				110
5	read and execute			101
4	read only					100
3	write and execute			011
2	write only					010
1	execute only				001
0	none						000
*/
};
class tree
{
public:
	node *root,*pwd;
	int next_inode;
	vector <node*> path;
	tree()
	{
	    root = new node;
	    pwd = new node;
		root -> inode = 0;
		root -> name = "root";
		root -> type = 0;
		root -> content = "";
		fill(root -> rwx,root -> rwx+3,"110");
		pwd = root;
		next_inode = 1;
		cout << "root/ ";
		root -> has_default_dir_permission = false;
	}
	bool dfs(node* dir,string find)
	{
		if (dir -> name == find)
		{
			path.insert(path.begin(),dir);
			return true;
		}
		int n = dir -> sub_dir_files.size();
		for (int i = 0; i < n; ++i)
		{
			if(dfs(dir -> sub_dir_files[i],find))
			{
				path.insert(path.begin(),dir);
				return true;
			}
		}
		return false;
	}
	bool processX(bool X,node *t)
	{
		if (X)
		{
			if (t -> rwx[0][2] == '1' || t -> rwx[1][2] == '1' || t->rwx[2][2]  == '1')
			{
				return true;
			}
		}
		return false;
	}
	void assignX(bool x,node *t)
	{
		if (x)
		{
			t -> rwx[0][2] = t -> rwx[1][2] = t -> rwx[2][2] = '1';
		}
	}
	void execute_recurse(node *t,string perms[],bool X)
	{
		bool x = processX(X,t);
		for (int i = 0; i < 3; ++i)
		{
			t -> rwx[i] = perms[i];
		}
		assignX(x,t);
		t -> has_default_dir_permission = false;
		for (int i = 0; i < t -> sub_dir_files.size(); ++i)
		{
			execute_recurse(t -> sub_dir_files[i],perms,X);
		}
	}
	void execute(string command)
	{
	    if (command == "pwd")
	    {
	        cwd();
	        return;
	    }
	    string base_command;
	    if (command.find(" ") == string :: npos)
        {
            if(command == "ls")
            {
                ls("");
                return;
            }
            base_command = command;
            if (base_command == "cd")
            	cout << "\nusage: cd <path>\n";
            else if (base_command == "touch")
	            cout << "\nusage: touch <filepath>\n";
	        else if (base_command == "mkdir")
	        	cout << "\nusage: mkdir <dir path>\n";
	        else if (base_command == "cat")
	        	cout << "\nusage cat <file path>\n";
	        else if (base_command == "chmod")
	        	cout << "\nusage: chmod [-R][--reference] <mode> <filelist>\n";
	        return;
        }
	    else base_command = command.substr(0,command.find(" "));
		for (int i = 0; i < valid_commands.size(); ++i)
		{
			if (valid_commands[i] == base_command)
			{
				if (base_command == "cd")
					cd(command.substr(3));
				else if (base_command == "ls")
					ls(command.substr(3));
				else if (base_command == "touch")
					touch(command.substr(6));
				else if (base_command == "mkdir")
					mkdir(command.substr(6));
				else if (base_command == "rm")
					rm(command.substr(3));
				else if(base_command == "cat")
					cat(command.substr(4));
				else if (base_command == "chmod")
					chmod(command.substr(6));
				else goto invalid;
				return;
			}
		}
		invalid:
		cout << "\nTerminal only supports the following commands:\n[";
		for (int i = 0; i < valid_commands.size(); ++i)
		{
			cout << valid_commands[i];
			if(i!=valid_commands.size() - 1)
                cout << ", ";
		}
		cout << "]\n";
	}
	void display_permissions(node* f)
	{
		for (int i = 0; i < 3; ++i)
		{
			bitset <3> s (string(f -> rwx[i]));
			cout << s.to_ulong();
		}
	}
	bool validate_permissions(string perms)
	{
		// All allowed: ugoaxX,rw-+=
		string allowed = "ugoa-+";
		for (int i = 0; i < perms.length(); ++i)
		{
			if (allowed.find(perms[i]) == string :: npos)
				return false;
			switch(perms[i])
			{
				case 'u': 	allowed = "go=+-";
							break;
				case 'g':	allowed = "uo=+-";
							break;
				case 'o':	allowed = "ug=+-";
							break;
				case 'a':	allowed = "+-=";
							break;
				case 'x':	allowed = ",";
							break;
				case 'X':	allowed = ",";
							break;
				case ',':	allowed = "ugoa";
							break;
				case 'r':	allowed = ",wxX+-";
							break;
				case 'w':	allowed = ",xX+-";
							break;
				case '-':
				case '+':
							allowed = "xXrw";
							break;
				case '=':	allowed = "xX,rw";
							break;
				default:	if (perms[i] == ' ')
							{
								cout << endl << "'chmod' command cannot take spaces. <mode> found to contain space(s)." << endl;
							}
							else cout << endl << perms[i] << " is not a valid <mode> specifier.\n";
							return false;
			}
		}
		return true;
	}
	vector <string> split(string filelist)
	{
		vector <string> files;
		if (filelist.empty())
		{
			cout << "\nError!\nusage: chmod [-R] <mode> <filelist> {filelist missing}\n";
			return files;
		}
		while(filelist.find(' ') != string :: npos)
		{
			files.push_back(filelist.substr(0,filelist.find(' ')));
			filelist.substr(filelist.find(' ') + 1);
		}
		files.push_back(filelist);
		return files;
	}
	void display_path(vector <node*> path)
	{
		int n = path.size();
		cout << endl;
		for (int i = 0; i < n; ++i)
		{
			cout << path[i] -> name << slash;
		}
		cout << endl;
	}
	void cwd()
	{
		path.clear();
		if(dfs(root,pwd->name))
			display_path(path);
		else cout << "\nInvalid path specified.\n";
	}
	void assign_perms(string trwx[],string p,char op,bool user,bool group,bool others)
	{
		string tp = "";
		tp = ((p.find('r') != string :: npos)?tp + '1':tp + '0');
		tp = ((p.find('w') != string :: npos) ?tp + '1':tp + '0');
		tp = ((p.find('x') != string :: npos) ?tp + '1':tp + '0');
		for (int j = 0; j < 3; j++)
		{
			if ((j == 0 && user == false) || (j == 1 && group == false) || (j == 2 && others == false))
				continue;
            string temp;
			switch(op)
			{
				case '=': trwx[j] = tp;
						  break;
				case '+':
				    temp = "";
					for (int i = 0; i < 3; ++i)
					{
						temp = ((tp[i] == '1')?temp + '1':temp + trwx[j][i]);
					}
					trwx[j] = temp;
					break;
				case '-':
					temp = "";
					for (int i = 0; i < 3; ++i)
					{
						temp = ((tp[i] == '0')?temp + '0':temp + trwx[j][i]);
					}
					trwx[j] = temp;
					break;
			}
		}
	}
	void cd(string dir_path)
	{
		if (dir_path == ".")
		{
			path.clear();
			if(dfs(root,pwd->name))
				display_path(path);
			else cout << "\nInvalid path specified.\n";
			return;
		}
		else if (dir_path == "/")
		{
			pwd = root;
			cout << "root/ ";
			return;
		}
		else if (dir_path == "..")
		{
			if (pwd -> name == "root")
			{
				cout << endl << "Cannot execute '..' for root directory" << endl;
				return;
			}
			path.clear();
			if(dfs(root,pwd->name))
			{
				pwd = path[path.size() - 2];
				path.pop_back();
				display_path(path);
			}
			else cout << "\nInvalid path specified.\n";
			return;
		}
		else
		{
			if (dir_path[dir_path.length() - 1] == slash)
				dir_path = dir_path.substr(0,dir_path.length() - 1);
			int l = dir_path.length(),i,j;
			int pos = dir_path.find(slash);
			if (pos == string :: npos)
			{
			    path.clear();
				if(dfs(root,dir_path) == false)
				{
					cout << "\nInvalid path specified.\n";
					return;
				}
				if(path[path.size() - 1] -> type == 1)
				{
					cout << "\nInvalid path. cd must take a directory path as argument, file path found.\n";
					return;
				}
				else
				{
					pwd = path[path.size() - 1];
					display_path(path);
					return;
				}
			}
			else
			{
				node *p = interpret_path(dir_path);
				if(p == NULL)
				{
				    return;
				}
				pwd = p;
				path.clear();
				dfs(root,pwd->name);
				display_path(path);
			}
		}
	}
	void r_ls(node* pwd)
	{
		cout << pwd -> name << "  ";
		if (pwd -> sub_dir_files.empty())
			return;
		cout << ":\n";
		for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
		{
			r_ls(pwd -> sub_dir_files[i]);
		}
		cout << "\n-------\n";
	}
	void ls(string option)
	{
		if ((pwd -> sub_dir_files).empty())
		{
			cout << "\nNo file or sub-directory under pwd\n";
			return;
		}
		if (option.empty())
		{
			for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
			{
				cout << pwd -> sub_dir_files[i] -> name << " ";
			}
			cout << endl;
		}
		else
		{
			if (option == "-1")
			{
				for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
				{
					cout << pwd -> sub_dir_files[i] -> name << endl;
				}
			}
			else if (option == "-l")
			{
				string filetype,Name;
				for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
				{
					Name = (pwd -> sub_dir_files[i]) -> name;
					if ((pwd -> sub_dir_files[i]) -> type == 0)
					{
						filetype = "Directory";
					}
					else
					{
						if (Name.find('.') != string :: npos)
						{
							filetype = Name.substr(Name.find('.')) + " file";
						}
						else filetype = "file";
					}
					cout << pwd -> sub_dir_files[i] -> inode << '\t';
					display_permissions(pwd -> sub_dir_files[i]);
					cout << '\t' << Name << '\t' << '\t' << filetype << endl;
				}
			}
			else if (option == "-R")
			{
				node* tpwd = pwd;
				r_ls(tpwd);
			}
			else
            {
                cout << "\nls expects only [-1, -R, -l] as its options.\n";
                return;
            }
		}
	}
	node* interpret_path(string dir_path)
	{
		if(dir_path.empty())
			return root;
		if (dir_path == ".")
			return pwd;
		if (dir_path == "..")
		{
			if (pwd -> name == "root")
            {
                cout << endl << "Invalid path specified. Path specified '..' for root directory" << endl;
                return NULL;
            }
			path.clear();
			if(dfs(root,pwd->name) == false)
			{
				cout << "\nInvalid path specified.\n";
				return NULL;
			}
			return path[path.size() - 2];
		}
		vector <string> dirs;
		string cur_dir = "";
		for (int i = 0; i < dir_path.length(); ++i)
		{
			if (dir_path[i] == slash)
			{
				dirs.push_back(cur_dir);
				cur_dir = "";
			}
			else cur_dir.push_back(dir_path[i]);
		}
		if(cur_dir.empty() == false)
        {
            dirs.push_back(cur_dir);
        }
		node *p = root;
		for (int i = 0; i < dirs.size(); ++i)
		{
			if(dirs[i].empty())
				p = root;
			else if (dirs[i] == ".")
				p = pwd;
			else if (dirs[i] == "..")
			{
				if (p -> name == "root")
                {
                    cout << endl << "Invalid path specified. Path specified '..' for root directory" << endl;
                    return NULL;
                }
				path.clear();
				if(dfs(root,p->name))
					p = path[path.size() - 2];
				else
                {
                    cout << "\nDirectory " << p -> name << "did not have sub-directory " << dirs[i] << " \n";
                    return NULL;
                }
			}
			else
			{
				bool change = false;
				for (int j = 0; j < p -> sub_dir_files.size(); ++j)
				{
					if (dirs[i] == (p -> sub_dir_files[j]) -> name)
					{
						if((p -> sub_dir_files[j]) -> type == 1)
							continue;
						p = p -> sub_dir_files[j];
						change = true;
						break;
					}
				}
				if (change == false)
				{
				    cout << "\nDirectory " << p -> name << " did not have " << dirs[i] << " as sub-directory.\n";
				    return NULL;
				}
			}
		}
		return p;
	}
	bool check_duplicate(string name,node* dir)
	{
		for (int i = 0; i < (dir -> sub_dir_files).size(); ++i)
		{
			if ((dir -> sub_dir_files[i]) -> name == name)
				return true;
		}
		return false;
	}
	void touch(string filename)
	{
		node* cur_node = pwd;
		if (filename.find(slash) != string :: npos)
		{
			cur_node = interpret_path(filename.substr(0,filename.rfind(slash)));
			if (cur_node == NULL)
			{
				return;
			}
			filename = filename.substr(filename.rfind(slash) + 1);
		}
		if (filename.empty())
		{
			cout << "File name cannot be empty\n";
			return;
		}
		if (check_duplicate(filename,cur_node))
		{
			filename.push_back('1');
			char rename_bit = '2';
			while(check_duplicate(filename,cur_node))
			{
				filename = filename.substr(0,filename.length() - 1);
				filename.push_back(rename_bit);
				rename_bit++;
			}
			cout << "\nFile/Directory name already exists. Rename to " << filename << "? [Y/N]";
			char ch;
			cin >> ch;
			if (ch == 'N' || ch == 'n')
			{
				cout << "\nFile creation canceled by user.\n";
				return;
			}
		}
		node* p = new node;
		p -> name = filename;
		p -> type = 1;
		p -> inode = next_inode++;
		string content = "",input;
		cout << "\nYou will now start entering content of this file, to stop input 'done' in a separate line:\n";
		getline(cin,input);
		while (input != "done")
		{
			content += input;
			getline(cin,input);
			content += "\n";
		}
		p -> content = content;
		for (int i = 0; i < 3; ++i)
		{
			p -> rwx[i] = cur_node -> rwx[i];
		}
		p -> has_default_dir_permission = true;
		(cur_node -> sub_dir_files).push_back(p);
		cout << "\nFile " << filename << " created.\n";
	}
	void mkdir(string dirname)
	{
		node* cur_node = pwd;
		if (dirname.find(slash) != string :: npos)
		{
			cur_node = interpret_path(dirname.substr(0,dirname.rfind(slash)));
			if (cur_node == NULL)
			{
				return;
			}
			dirname = dirname.substr(dirname.rfind(slash) + 1);
		}
		if (dirname.empty())
		{
			cout << "Directory name cannot be empty\n";
			return;
		}
		if (check_duplicate(dirname,cur_node))
		{
			dirname.push_back('1');
			char rename_bit = '2';
			while(check_duplicate(dirname,cur_node))
			{
				dirname = dirname.substr(0,dirname.length() - 1);
				dirname.push_back(rename_bit);
				rename_bit++;
			}
			cout << "\nDirectory/File name already exists. Rename to " << dirname << "? [Y/N]";
			char ch;
			cin >> ch;
			if (ch == 'N' || ch == 'n')
			{
				cout << "\nDirectory creation canceled by user.\n";
				return;
			}
		}
		node* p = new node;
		p -> name = dirname;
		p -> type = 0;
		p -> inode = next_inode++;
		p -> content = "";
		for (int i = 0; i < 3; ++i)
		{
			p -> rwx[i] = cur_node -> rwx[i];
		}
		p -> has_default_dir_permission = true;
		(cur_node -> sub_dir_files).push_back(p);
		cout << "\nDirectory " << dirname << " created.\n";
	}
	void cat(string filename)
	{
		node* cur_node = pwd;
		if (filename.find(slash) != string :: npos)
		{
			cur_node = interpret_path(filename.substr(0,filename.rfind(slash)));
			if (cur_node == NULL)
			{
				return;
			}
			filename = filename.substr(filename.rfind(slash) + 1);
		}
		if (filename.empty())
		{
			cout << "File name cannot be empty\n";
			return;
		}
		if (cur_node == NULL)
		{
			path.clear();
			if (dfs(root,filename))
			{
				cur_node = path[path.size() - 1];
			}
			else
			{
                cout << "\nInvalid path specified\n";
                return;
			}
		}
		for (int i = 0; i < (cur_node -> sub_dir_files).size(); ++i)
		{
			if ((cur_node -> sub_dir_files[i]) -> type == 1 && (cur_node -> sub_dir_files[i]) -> name == filename)
			{
				if (cur_node -> sub_dir_files[i] -> rwx[0][0] == '0')
				{
					cout << "Access denied. File doen't have read permissions.\n";
					return;
				}
				if (cur_node -> sub_dir_files[i] -> content.empty())
				{
				    cout << cur_node -> sub_dir_files[i] -> name << " has no contents\n";
				    return;
				}
				cout << "The content of the file is:\n" << cur_node -> sub_dir_files[i] -> content << endl;
				return;
			}
		}
		cout << "Error! Path specified does not exist.\n";
	}
	void rm(string filename)
	{
		if(filename.empty())
		{
			cout << "No command detected.\n";
			return;
		}
		bool force = false;
		if (filename.length() > 3 && filename[0] == '-' && filename[1] == 'f' && filename[2] == ' ')
		{
			force = true;
			filename = filename.substr(filename.find(' ') + 1);
		}
		if(filename.empty())
		{
			cout << "No file name detected.\n";
			return;
		}
		if (filename == "*")
		{
			if (force)
			{
				pwd -> sub_dir_files.clear();
				cout << "All files and sub-directories deleted.\n";
				return;
			}
			for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
			{
				if (pwd -> sub_dir_files[i] -> type == 0)
				{
					if (pwd -> sub_dir_files[i] -> rwx[0][1] == 0)
					{
						char ch;
						cout << "Files which are write-protected exist in this directory. Delete? [Y/N]: ";
						cin >> ch;
						repeat:
						if (ch == 'n' || ch == 'N')
						{
							cout << "No file/directory deleted. Operation canceled by user.\n";
							return;
						}
						else if (ch == 'y' || ch == 'Y')
						{
							break;
						}
						else
						{
							cout << "'" << ch <<"' is an invalid entry. Try 'y' or 'n':";
							cin >> ch;
							goto repeat;
						}
					}
				}
			}
			pwd -> sub_dir_files.clear();
			cout << "All files and sub-directories deleted.\n";
			return;
		}
		node* cur_node = pwd;
		if (filename.find(slash) != string :: npos)
		{
			cur_node = NULL;
			cur_node = interpret_path(filename.substr(0,filename.rfind(slash)));
			if (cur_node == NULL)
			{
				return;
			}
			filename = filename.substr(filename.rfind(slash) + 1);
		}
		if (filename.empty())
		{
			cout << "File/Directory name cannot be empty.\n";
			return;
		}
		for(int i = 0; i < cur_node -> sub_dir_files.size(); i++)
		{
			if (cur_node -> sub_dir_files[i] -> name == filename)
			{
				cur_node = cur_node -> sub_dir_files[i];
				break;
			}
		}
		if (cur_node == NULL)
		{
			cout << "\nInvalid path specified\n";
            return;
		}
		if (cur_node -> type == 0)
		{
			if (cur_node == root)
			{
				cout << "\nRoot directory deletion prohibited.\nDelete everything under root instead? [Y/N]: ";
				char ch;
				cin >> ch;
				switch(ch)
				{
					case 'y':
					case 'Y':	(root -> sub_dir_files).clear();
								next_inode = 1;
								cout << "\nSuccess. \n";
								break;
					case 'N':
					case 'n': 	cout << "\nOperation canceled by user.\n";
								break;
				}
				return;
			}
			for(int i = 0; i < pwd -> sub_dir_files.size(); i++)
            {
                if (force == false && (pwd -> sub_dir_files[i]) -> rwx[0][1] == '0')
                {
                    cout << "\nAll files and directories in this directory will be deleted, including write-protected ones. \nContinue? [Y/N]:";
                    char ch;
                    cin >> ch;
                    if (ch == 'N' || ch == 'n')
                    {
                        cout << "\nNo file/directory deleted. Operation canceled by user.\n";
                        return;
                    }

                    break;
                }
            }
            node* parent;
            path.clear();
            if(dfs(root,cur_node -> name))
            {
                parent = path[path.size() - 2];
            }
            else
            {
                cout << "\nInvalid path specified.\n";
                return;
            }
			for (int i = 0; i < parent -> sub_dir_files.size(); ++i)
			{
				if ((parent -> sub_dir_files[i]) -> name == cur_node -> name)
				{
                    (parent -> sub_dir_files).erase((parent -> sub_dir_files).begin() + i);
					break;
				}
			}
			cout << "\nDirectory deleted.\n";
			if (pwd -> name == filename)
			{
				pwd = parent;
				path.pop_back();
				display_path(path);
			}
			return;
		}
		if (force == false && cur_node -> rwx[0][1] == '0')
		{
			cout << "\nFile is write-protected. Continue? [Y/N]:";
			char ch;
			cin >> ch;
			if (ch == 'N' || ch == 'n')
			{
				cout << "\nNo file deleted. Operation canceled by user.\n";
				return;
			}
		}
		node* parent;
        path.clear();
        if(dfs(root,cur_node -> name))
        {
            parent = path[path.size() - 2];
        }
        else
        {
            cout << "\nInvalid path specified.\n";
            return;
        }
		for (int i = 0; i < parent -> sub_dir_files.size(); ++i)
        {
            if ((parent -> sub_dir_files[i]) -> name == cur_node -> name)
            {
                (parent -> sub_dir_files).erase((parent -> sub_dir_files).begin() + i);
                break;
            }
        }
		cout << "\nFile deleted.\n";
		return;
	}
	void chmod(string command)
	{
		if (command.empty())
		{
			cout << "\nchmod expects a non-empty command.\n";
			return;
		}
		bool recurse = false;
		if (command.find("-R") == 0 && command.length() > 3 && command[2] == ' ')
		{
			recurse = true;
			command = command.substr(3);
		}
		if (command.find(' ') == string::npos)
		{
			cout << "\nError!\nusage: chmod [-R][--reference] <mode> <filelist>\n";
			return;
		}
		string perms = command.substr(0,command.find(' '));
		string trwx[3] = {"---","---","---"};
		bool reference = false, X = false;
		if (perms.length() == 3 && perms[0] >= 48 && perms[0] <= 57)
		{
			if (perms[1] < 48 || perms[1] > 57 || perms[2] < 48 || perms[2] > 57)
			{
				cout << "\n<mode> specifications invalid.\n";
				return;
			}

			for (int i = 0; i < 3; ++i)
			{
			    bitset <3> b(perms[i] - 48);
				trwx[i] = b.to_string();
			}
			command = command.substr(command.find(' ') + 1);
		}
		else if (perms.length() > 12 && perms.find("--reference=") == 0)
		{
			reference = true;
			command = command.substr(perms.find('=') + 1);
		}
		else
		{
			if (validate_permissions(perms) == false)
				return;
			bool user = false,group = false,others = false, X = false;
			string mode = perms;
			command = command.substr(command.find(' ') + 1);
			if (command.empty())
			{
				cout << "\nFile name not specified\n";
			}
			for (int i = 0; i < mode.length(); ++i)
			{
				if (mode[i] == 'u')
					user = true;
				else if (mode[i] == 'a')
					user = group = others = true;
				else if (mode[i] == 'g')
						group = true;
				else if (mode[i] == 'o')
					others = true;
				else if (mode[i] == '+' || mode[i] == '-' || mode[i] == '=')
				{
					if (i == 0 && mode[i] != '=')
					{
						user = group = others = true;
					}
					string p = "";
					char op = mode[i];
					i++;
					while(i < mode.length() && mode[i] != ',')
					{
						if (mode[i] == 'X')
						{
							X = true;
							i++;
							continue;
						}
						p += mode[i];
						i++;
					}
					if(p.empty() == false)
					{
						assign_perms(trwx,p,op,user,group,others);
						user = group = others = false;
					}
				}
				else if (mode[i] == ',')
				{
					user = group = others = false;
				}
				else
				{
					cout << "\nSpecified <mode> incorrect\n";
					return;
				}
			}
		}
		/*
		Handle cases for:
		1. X
		2. -R
		3. --reference
		*/
		vector <string> files = split(command);
		if (files.empty())
			return;
		if (reference)
		{
			if (files.size() != 2)
			{
				cout << "\nError! usage: chmod [--reference]=filename1 filename2\n";
				return;
			}
			int reference_file = 0;
			bool success = false;
			for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
			{
				if (pwd -> sub_dir_files[i] -> name == files[0])
				{
					reference_file = i;
					success = true;
				}
			}
			if (success == false)
			{
				cout << "\nCouldn't find " << files[0] << " in pwd.\n";
				return;
			}
			success = false;
			for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
			{
				if (pwd -> sub_dir_files[i] -> name == files[1])
				{
					for (int j = 0; j < 3; ++j)
					{
						pwd -> sub_dir_files[i] -> rwx[j] = pwd -> sub_dir_files[reference_file] -> rwx[j];
					}
					pwd -> sub_dir_files[i] -> has_default_dir_permission = false;
					success = true;
				}
			}
			if (success == false)
			{
				cout << "\nCouldn't find " << files[1] << " in pwd.\n";
				return;
			}
			cout << "\nMode(s) of " << files[0] << " assigned to " << files[1] << endl;
			return;
		}
		if (recurse)
		{
			if (files.size() != 1)
			{
				cout << "\nError! usage: chmod [-R] dirname\n";
				return;
			}
			bool success = false;
			int dir;
			for (int i = 0; i < pwd -> sub_dir_files.size(); ++i)
			{
				if (pwd -> sub_dir_files[i] -> type == 0 && pwd -> sub_dir_files[i] -> name == files[0])
				{
					success = true;
					dir = i;
					break;
				}
			}
			if (success == false)
			{
				cout << "\nDirectory name " << files[0] << " not found.\n";
				return;
			}
			execute_recurse(pwd -> sub_dir_files[dir],trwx,X);
			cout << "\nFile modes assigned\n";
			return;
		}
		vector <string> skipped_files;
		for (int i = 0; i < files.size(); ++i)
		{
			bool success = false;
			for (int j = 0; j < pwd -> sub_dir_files.size(); ++j)
			{
				if (pwd -> sub_dir_files[j] -> name == files[i])
				{
					success = true;
					bool x = processX(X,pwd -> sub_dir_files[j]);
					for (int k = 0; k < 3; ++k)
					{
						for (int h = 0; h < 3; ++h)
						{
							if (trwx[k][h] == '-')
								continue;
							pwd -> sub_dir_files[j] -> rwx[k][h] = trwx[k][h];	
						}
					}
					assignX(x,pwd -> sub_dir_files[j]);
				}
			}
			if (success == false)
			{
				skipped_files.push_back(files[i]);
			}
		}
		if (skipped_files.empty() == false)
		{
			cout << "\nThe following files/directories were not found in pwd:\n";
			for (int i = 0; i < skipped_files.size(); ++i)
			{
				cout << skipped_files[i] << endl;
			}
			cout << endl;
			return;
		}
		cout << "\nModes set.\n";
	}
};
int main()
{
	tree t;
	string command = " ";
	init_valid_commands();
	while(1)
	{
	    if(command.empty() == false)
            cout << "\n$ ";
        getline(cin,command);
        if(command.empty() == false)
        {
            t.execute(command);
        }
        if(command == "exit")
            break;
	}
	return 0;
}
