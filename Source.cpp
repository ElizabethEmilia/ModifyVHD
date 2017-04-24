/*
	ModifyVHDext2
    Copyright (C) Miyuki  2014

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Please contact me at <miyuki@miyuki.hk> or <listencpp@gmail.com>
*/

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <cctype>
using namespace std;
//#define DEBUG
#pragma warning (disable: 4996)

//VHD footer
struct footer{
	char			   cookie[8];
	unsigned long	   features;
	unsigned long	   file_format_ver;
	unsigned long long data_offset;
	unsigned long	   time_stamp;
	char			   creator_application[4];
	char			   creator_version[4];
	char		       creator_host_os[4];
	unsigned long long original_size;
	unsigned long long current_size;
	unsigned long	   geometry;
	unsigned long      disk_type;
	unsigned long      checksum;
	char			   unique_id[16];
	char			   state;
	char			   reserved[427];
};
footer vhd_footer;
//Parameter management
struct para{
	string name;
	string value;
};
vector<para> parameters;
bool         quiet=false;

inline para * pfind_para(char* key){
	for (int i = 0; i < parameters.size(); i++)
		if (parameters[i].name == key)
			return &parameters[i];
	return nullptr;
}

inline para padd_para(const char * arg){
	bool cp = false;
	para p;
	for (int i = 0; arg[i]; ++i){
		if (arg[i] == '='){
			cp = true;
			continue;
		}
		(!cp ? p.name : p.value) += arg[i];
	}
	return p;
}

inline string pget_value(char * key){
	return pfind_para(key)->value;
}

inline void pread_args(int argc, const char** argv){
	for (int i = 1; i < argc; i++)
		parameters.push_back(padd_para(argv[i]));
}

inline bool pexist(char * key){
	return pfind_para(key) != nullptr;
}

void help(){
	cout << "Modify VHD Extended Version 2\nCopyright (C) 2015 Miyuki\nThis program is licensed under GNU General Public License Version 3\n"
		<< " Usage: {{{-v={file} -b={file} [-s={number}] [-i]} | -batch={file} | -rac[={file}]} [-q]} | -help | -gpl-license\n"
		<< endl
		<< "  -b=<file path>      Specify source binary file (required)\n"
		<< "  -batch=<file path>  Specify a batch file to process a lot of operations\n"
		<< "  -gpl-license        Show GPL license information\n"
		<< "  -help               Show this help content\n"
		<< "  -i                  Show VHD Information (VHD footer)\n"
		<< "  -q                  Run program without output\n"
		<< "  -rac=<asm file>     Read destination section from an ASM file\n"
		<< "  -s=<section>        Specify section number to write in (dafault for 0)\n"
		<< "  -v=<file path>      Specify VHD file to write data in (required)\n"
		<< endl
		<< " Notice:\n"
		<< "  -rac parameter has higher priority than -s parameter. BIN file must have the same name as ASM file if the ASM file is not specified. If you want to use -rac,"
		   " add comment to the first line of the ASM file as below:\n"
		<< "   ; Section=100\n"
		<< " Which means write BIN to section #100."
		<< endl;
}

void gpl_license(){
	cout << "Modify VHD Extend Version 2\nCopyright (C) 2015 Miyuki\n"
		<< endl
		<< "This program is free software : you can redistribute it and / or modify "
		<< "it under the terms of the GNU General Public License as published by "
		<< "the Free Software Foundation, either version 3 of the License, or "
		<< "(at your option) any later version."
		<< endl
		<< "This program is distributed in the hope that it will be useful,"
		<< "but WITHOUT ANY WARRANTY; without even the implied warranty of"
		<< "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the"
		<< "GNU General Public License for more details."
		<< endl
		<< "You should have received a copy of the GNU General Public License"
		<< "along with this program.If not, see <http://www.gnu.org/licenses/>."
		<< endl
		<< "Please contact me at <miyuki@miyuki.cc> or <listencpp@gmail.com>"
		<< endl << endl
		<< "Compile time:" << __DATE__ << ", " << __TIME__;
}

template <class T>
inline T reverse_bits(T& num){
	char * r = (char*)&num;
	for (int i = 0; i < sizeof(num) / 2; i++)
		std::swap(r[i], r[sizeof(num) - i - 1]);
	return num;
}

void read_vhd_footer(const char * _vhd){
	ifstream vhd_file(_vhd, ios_base::binary);
	if (!vhd_file){
		cout << "VHD file not found.";
		exit(8);
	}
	vhd_file.seekg(-512, ios_base::end);
	vhd_file.read((char*)&vhd_footer, sizeof(vhd_footer));
	reverse_bits(vhd_footer.features);
	reverse_bits(vhd_footer.file_format_ver);
	reverse_bits(vhd_footer.data_offset);
	reverse_bits(vhd_footer.time_stamp);
	reverse_bits(vhd_footer.original_size);
	reverse_bits(vhd_footer.current_size);
	reverse_bits(vhd_footer.geometry);
	reverse_bits(vhd_footer.disk_type);
	reverse_bits(vhd_footer.checksum);
}

void output_information(){
	read_vhd_footer(pget_value("-v").c_str());
	cout << "Disk information:\n"
		<< "  File path:	      " << pget_value("-v").c_str() << endl
		<< "  Cookie:		      " << vhd_footer.cookie << endl
		<< "  Creator Application:" << vhd_footer.creator_application[0] << vhd_footer.creator_application[1] << vhd_footer.creator_application[2] << endl
		<< "  Create Host OS:     " << vhd_footer.creator_host_os << endl
		<< "  Time stamp:         " << vhd_footer.time_stamp << endl
		<< "  Capacity:           " << vhd_footer.original_size / 1024 / 1024 << " MB"  << endl
		<< "  Disk type:          " << 
		[](int type){
			switch (type){
			case 0:
				return "None";
			case 2:
				return "Fixed Hard Disk";
			case 3:
				return "Dynamic Hard Disk";
			case 4:
				return "Differencing Hard Disk";
			case 1:
			case 5:
			case 6:
				return "--Reserved--";
			}
			return "ERROR";
	}(vhd_footer.disk_type) << endl;
}

void vhd_write(const char* _vhd, const char* _bin, int section){
	read_vhd_footer(_vhd);
	//如果不是固定大小的磁盘就数组错误信息
	if (vhd_footer.disk_type != 2){
		cout << "Given VHD file is not fixed VHD file, which is not able to modify by this program.";
		exit(9);
	}
	fstream vhd_file(_vhd, ios_base::in | ios_base::out | ios_base::binary);
	ifstream bin_file(_bin, ios_base::binary);
	if (!quiet) cout << "Opening VHD file...\n";
	if (!vhd_file){
		if (!quiet) cout << "VHD file not found.";
		exit(2);
	}
	if (!quiet) cout << "Opening VHD file...\n";
	if (!bin_file){
		if (!quiet) cout << "BIN file not found.";
		exit(3);
	}
	if (!quiet) cout << "Modifying VHD file...\n";
	int current_pos = -1;
	vhd_file.seekp(section * 512, ios_base::beg);
	while (!bin_file.eof()){
		vhd_file.put(bin_file.get());
		current_pos++;
	}
	//用0补齐512字节
	for (int i = current_pos % 513; i < 512; i++)
		vhd_file.put(0);
	vhd_file.close();
	bin_file.close();
}

void rac_command(int& section){
	char str[50];
	string _asm_file = [](){
		if (pget_value("-rac") != "")
			return pget_value("-rac");
		//如果没有指定asm文件名称则寻找去掉.bin的文件
		string s=pget_value("-b");
		s.erase(s.end() - 3, s.end());
		return s;
	}();
	fstream asm_file(_asm_file.c_str());
	if (!asm_file){
		cout << "Information: ASM file not found.";
		return;
	}
	asm_file.getline(str, 50);
	auto trim = [](char * s){
		char str_tmp[50] = "";
		for (int i = 0, j = 0; s[i]; ++i)
			if (s[i] != ' ' && s[i] != ';')
				str_tmp[j++] = tolower(s[i]);
		strcpy(s, str_tmp);
		return s;
	};
	para p = padd_para(trim(str));
	if (p.name != "section"){
		cout << "Information: No 'section' appears in the first line of comment in ASM file found.";
		return;
	}
	section = atoi(p.value.c_str());
}

void batch(const char * app_path){
	char str[256];
	//Batch file格式和调用程序的格式一样
	fstream batch_file(pget_value("batch").c_str());
	if (!batch_file){
		cout << "Batch file not found.";
		exit(6);
	}
	while (batch_file.eof()){
		batch_file.getline(str, 256);
		system(str);
	}
	exit(0);
}

int main(int argc, const char** argv){
	pread_args(argc, argv);
#ifdef DEBUG
	help();
	cout << "Parameters" << endl;
	for (auto e : parameters)
		cout << e.name << ": " << e.value << endl;
	getchar();
#endif
	//扇区编号： 如果存在-s参数就使用-s提供的值，否则使用默认的0
	int    section_number = pexist("-s") ? atoi(pget_value("-s").c_str()) : 0; //扇区编号
	// 获得参数
	if (pexist("-help")){
		help();
		return 0;
	}
	else if (pexist("-gpl-license")){
		gpl_license();
		return 0;
	}
	quiet = pexist("-q");
	if (pexist("-batch")){
		batch(argv[0]);
		return 0;
	}
	else if (pexist("-rac")){
		rac_command(section_number);
	}
	// -v是必须的参数。
	// -b是必须的参数
	if (!pexist("-b") || !pexist("-v")){
		help();
		return 1;
	}
	// 如果指定了-i参数则输出信息
	if (pexist("-i"))
		output_information();
	//写入信息
	vhd_write(pfind_para("-v")->value.c_str(),  pfind_para("-b")->value.c_str(), section_number);
	return 0;
}