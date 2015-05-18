#include <sstream>
#include <iostream>
#include <fstream>  //Чтение и запись
#include <string>
#include <boost/filesystem.hpp>  //BOOST::FILESYSTEM с помощью которого считываем директорию
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp> //BOOST Serialization
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "keccak.h"
#include <stdio.h>

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;
using namespace std;

namespace fs = boost::filesystem;
std::string test;


//Данные будем записывать в структуру, а структуру копировать в вектор
struct Fileinfo {
	std::string path;
	std::string hash;
	int size;
	std::string flag;
};



string output1;

void get_dir_list(fs::directory_iterator iterator, std::vector<Fileinfo> * vec_finfo) {  //выводит список файлов и папок в директории
	Fileinfo finfo; //объявление структуры, в которую будем записывать данные и складывать их в вектор
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) { //если наткнулись на папку, то рекурсивно запускаем эту же функцию для этой папки
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo);

		}
		else //а если нет, то записываем в структуру имя, размер, хеш, и флажок (понадобится чуть позже, когда будем искать изменения в файлах)
		{


			stringstream result;
			string a;
			ifstream myfile;
			myfile.open(iterator->path().string(), ios::binary);
			result << myfile.rdbuf();
			a = result.str();

			//Hash file
			Keccak keccak;
			output1 = keccak(a);



	

			finfo.path = iterator->path().string();
			std::replace(finfo.path.begin(), finfo.path.end(), '\\', '/'); //исправляем косяк с \ перед файлом в пути, так работает filesystem :-(
			finfo.size = fs::file_size(iterator->path());
			finfo.hash = output1;
			finfo.flag = "NEW";
			vec_finfo->push_back(finfo);



		}

	}
}


vector<Fileinfo> compare_lists(vector<Fileinfo> newfl, vector<Fileinfo> oldfl) {
	string value1;
	string value2;
	string value3;
	std::ifstream ofs("result.bsr");
	boost::archive::text_iarchive oa(ofs);
	std::string test;
	Fileinfo it1;
	while (true) {
		try
		{

			for (int i = 1; i < 1000000000000000;) {  //Запись, просто по сделанным методам протобафа
				oa >> it1.path;
				oa >> std::to_string(it1.size);
				oa >> it1.hash; 

				oldfl.push_back(it1);
			}

		}
		catch (boost::archive::archive_exception& ex)
		{
			break;
		}
	}
	ofs.close();

	//Compare
	for (vector<Fileinfo>::iterator itnew = newfl.begin(); itnew < newfl.end(); itnew++) {
		for (vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
			if (itnew->hash == itold->hash) {
				itnew->flag = "UNCHANGED";
				oldfl.erase(itold);
				break;
			}
			if (itnew->hash != itold->hash) {
				itnew->flag = "CHANGED";
				oldfl.erase(itold);
				break;
			}
		}
	}
	for (vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
		itold->flag = "DELETED";
		newfl.push_back(*itold);
	}

	//Output
	for (Fileinfo data : newfl){
		cout << data.path << endl
			<< data.size << endl
			<< data.hash << endl
			<< data.flag << endl;
	}
//	ofs.close();
	return newfl;
}

//Записываем через протобаф. Filelist - внешняя структура, в которую кладем элементы Filep
void save2bsr(std::string filename, std::vector<Fileinfo> vec_finfo) {
	
	cout << "Do you want save as json?" << endl;
	string x;
	
	
	if (x == "yes"){
	boost::property_tree::ptree tree;

	int i = 1;
	
	for (Fileinfo it : vec_finfo) {
		tree.put("File" + std::to_string(i), it.path);
		tree.put("Size" + std::to_string(i), it.size);
		tree.put("Hash" + std::to_string(i), it.hash);
		i++;
		
	}

	
	int c = 0;
	std::ofstream ofs("result.bsr"); //имя файла
	boost::archive::text_oarchive oa(ofs);
	std::string hash = output1;
	for (Fileinfo it : vec_finfo) {  //Запись, просто по сделанным методам протобафа
		oa << it.path;
		cout << it.path << endl;
		oa << std::to_string(it.size);
		cout << it.size << endl;
		oa << it.hash; // добавить позже
		cout << it.hash << endl;
		
		c = c + it.size;
	}

        //Output of general size
         cout << "General size:" <<
                  c << endl;
	}

	ofs.close();
}

int main() {

	string path, dirpath;

	cout << "What do you want?" << endl <<
		"press 1 - save new data" << endl << "press 2 - check new data" << endl;
	string status; // 1 or 2
	getline(cin, status);

	cout << "Folder path:" << std::endl;
	getline(cin, path);

	//If path doesn't exists stop the programm
	if (!(fs::exists(path))) {
		cout << "This directory doesn't exist. Please, try again.";
		status = "null";

	}

	else{
		vector<Fileinfo> vec_finfo;
		vector<Fileinfo> vec_finfo_old;
		vector<Fileinfo> vec_finfo_new;
		fs::directory_iterator home_dir(path);
		get_dir_list(home_dir, &vec_finfo);


		if (status == "1"){
			cout << endl;

			save2bsr("result.json", vec_finfo);
			std::cin.clear();
			fflush(stdin);

		}


		if (status == "2") {
			vector<Fileinfo> old;
			compare_lists(vec_finfo, old);
			
		}
	}
	cin.get();
	return 0;


}
