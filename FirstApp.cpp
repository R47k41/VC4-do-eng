#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <Windows.h>
#include <vector>
#include <set>
#include <math.h>
#include <algorithm>
#include <iterator>
#include <utility>
#include <boost/filesystem.hpp>
//#include <boost/algorithm/string.hpp>
namespace fs = boost::filesystem;

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;
using std::ostream;
using std::inserter;
using std::ostream_iterator;
using std::replace;
using fs::file_size;
using fs::exists;
using fs::is_directory;
using fs::path;
using fs::directory_iterator;
using fs::directory_entry;
using fs::filesystem_error;
using std::ios_base;

//Определения типов:
//функция принимающая символ
//Стурктура файла
typedef std::pair<std::string, std::string> TFileRec;
//Массив имен файлов
typedef std::set<TFileRec> TFileArr;
//массив путей
using TPathArr = std::set<fs::path>;

//установка локали
inline void SetRusLang(int cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); };

//***Источник - eng версия
//***Приемник - jpn версия

//**** Основная суть программы заменить файлы в японской версии на
//файлы английской версии

//функция преобразования строки в нижний регистр
string set_chr_case(const string& str, int(*)(int));

//1. устанавливаем путь к источнику и путь к приемнику - это должны быть папки
fs::path GetUsrPath(const string& msg) noexcept(false);

//2. получение списка файлов в приемнике: путь + имя
TFileArr getFilesArraybyPath(const fs::path& p);

//функция получения пути для файла

//2.1 создаем файлы лога для копирования файлов:
//для каждой папки из приемника - свой файл лога по имени папки
//заносим туда информацию о замененных файлах:
//	имя + размер файла в источнике + размер в приемнике + путь до файла в приемнике

//2.2 создаем файл лога для файлов которые не найдены в источнике

//3. Для каждого файла из источника ищем его в приемнике + запись в логе
//сравнение имен файлов:
bool compare_files_names(const string& file1, const string& file2, int(*fnc)(int) = std::tolower) noexcept(true);
//поиск файла в папке источнике
fs::path getFileInSrc(const string& fname, const fs::path& dir) noexcept(false);
//копирование для одного файла
void Copy4File(const path& pfile, const fs::path& src_dir, ostream& log) noexcept(false);
//выполнение проверок для указанной директории:
bool checkDir(const fs::path& p, string& err_msg) noexcept(true);
//копирование массива файлов
void CopyFiles(const TFileArr& arr, const fs::path& src_dir) noexcept(true);
//отобразить файлы каталога:
void ShowTree(const fs::path& fpath) noexcept(false);

int main(void)
{
	SetRusLang(1251);
	try
	{
		//получение директории файлов приемника
		path SrcDir = GetUsrPath("Укажите путь к источнику: ");
		//path SrcDir("E:\\Rus_Make\\Dead rising\\Dead Rising 2\\test\\PC.RU\\");
		if (SrcDir.filename().empty()) return 0;
		path DestDir = GetUsrPath("Укажите путь к приемнику: ");
		//path DestDir("E:\\Rus_Make\\Dead rising\\Dead Rising Off Record\\test\\pc_rus");
		if (DestDir.filename().empty()) return 0;
		TFileArr DestFiles;
		DestFiles = getFilesArraybyPath(DestDir);
		for (auto a : DestFiles)
			cout << a.second << " в папке: " << a.first << endl;
		CopyFiles(DestFiles, SrcDir);
	}
	catch (string err)
	{
		cout << err.c_str() << endl;
		return 1;
	}
	catch (std::runtime_error err)
	{
		cout << err.what() << endl;
		return 1;
	}
	catch (fs::filesystem_error err)
	{
		cerr << err.what() << endl;
		return 1;
	}
	catch (...)
	{
		cout << "Не обработанное исключение!" << endl;
		return 1;
	}
	return 0;
}

//функция установки регистра для строки:
string set_chr_case(const string& str, int(*fnc)(int))
{
	using std::transform;
	if (str.empty()) return string{};
	if (fnc != tolower and fnc != toupper)
	{
		cerr << "Не верное применение функции" << endl;
		return string{};
	}
	string result;
	transform(str.begin(), str.end(), inserter(result, result.begin()), fnc);
	return result;
}

//Функция получения пути до файлов от пользователя
path GetUsrPath(const string& msg) noexcept(false)
{
	using std::cin;
	path result;
	do
	{
		string fpath;
		cout << msg << endl;
		getline(cin, fpath);
		if (!cin)
		{
			throw "Ввод прекращен. Программа завершит свою работу!";
		}
		result = path(fpath);
		if (is_directory(result) == false)
			cout << "Ошибка при указании директории! Повторите ввод!" << endl;
	} while (!is_directory(result) or !cin);
	return result;
}

//функция заполнения списка файлами из директории
TFileArr getFilesArraybyPath(const fs::path& p) noexcept(false)
{
	TFileArr result;
	if (p.filename().empty())
		throw ("Указан пустой путь!");
	if (!exists(p))
	{
		string msg;
		msg.append("Указанный путь: ").append(p.filename().string()).append(" не существует!");
		throw msg;
	}
	//выполняем проход по всем файлам директории
	for (directory_entry dir : directory_iterator(p))
	{
		//если это директория
		if (is_directory(dir))
		{
			TFileArr tmpArr;
			tmpArr = getFilesArraybyPath(dir.path());
			std::copy(tmpArr.begin(), tmpArr.end(), std::inserter(result, result.end()));
		}
		else
		{
			TFileRec tmpFile;
			tmpFile.first = dir.path().parent_path().string();
			tmpFile.second = dir.path().filename().string();
			result.insert(tmpFile);
		}
	}

	return result;
};

void ShowTree(const fs::path& fpath) noexcept(false)
{
	if (exists(fpath) && is_directory(fpath))
	{
		for (directory_entry& d : directory_iterator(fpath))
		{
			if (is_directory(d))
			{
				cout << d.path() << endl;
				ShowTree(d);
			}
			else
			{
				cout << '\t' << '\\' << d.path() << endl;
			}
		}
	}
	else
		throw ("Ошбка в указанном пути: " + fpath.string());
};
//сравнение имен файлов
bool compare_files_names(const string& file1, const string& file2, int(*fnc)(int)) noexcept(true)
{
	return set_chr_case(file1, fnc) == set_chr_case(file2, fnc) ? true : false;
}
//поиск файла в папке источнике
fs::path getFileInSrc(const string& fname, const fs::path& dir) noexcept(false)
{
	for (directory_entry f : directory_iterator(dir))
	{
		if (is_directory(f))
			return getFileInSrc(fname, f.path());
		else
			//проверяем совпадение имен:
			if (compare_files_names(f.path().filename().string(), fname))
				return f.path();
	}
	return fs::path{};
}

//копирование для одного файла
void Copy4File(const path& pfile, const fs::path& src_dir, ostream& log) noexcept(false)
{
	using std::abs;
	fs::path p = getFileInSrc(pfile.filename().string(), src_dir);
	//если файл не найден:
	if (p.empty())
		log << "Файл " << pfile.filename().string() << " не найден в каталоге: " << src_dir.string();
	else
	{
		//копирование файла в папку назначения:
		uintmax_t old_sz = file_size(pfile);
		uintmax_t new_sz = file_size(p);
		int percent_sz = (old_sz - new_sz) / old_sz;
		if (std::abs(percent_sz) > 35)
			log << "***WARNING***\t";
		log << "Файл: " << pfile.filename().string() << "(" << file_size(pfile);
		log << "/" << file_size(p) << ") заменен файлом из каталога: " << p.parent_path().string();
		try
		{
			//fs::copy_file(p, pfile, fs::copy_option::overwrite_if_exists);
		}
		catch (fs::filesystem_error err)
		{
			log << err.what() << endl;
		}
	}
	log << endl;
}

//выполнение проверок для указанной директории:
bool checkDir(const fs::path& p, string& err_msg) noexcept(true)
{
	//проверки для пути поиска
	if (p.filename().empty())
	{
		err_msg = "Указан пустой путь поиска";
		return false;
	}
	if (!is_directory(p))
	{
		err_msg = "Ошибка в указанном пути к источнику: " + p.filename().string();
		return false;
	}
	else if (!fs::exists(p))
	{
		err_msg = "Указанный каталог: " + p.filename().string() + " не найден!";
		return false;
	}
	return true;
}

//копирование массива файлов
void CopyFiles(const TFileArr& arr, const fs::path& src) noexcept(true)
{
	//если массив файлов пуст - выходим
	if (arr.empty())
	{
		cerr << "Указан пустой массив!" << endl;
		return;
	}
	//выполняем проверку введенног пути:
	string err;
	if (checkDir(src, err) == false)
	{
		cerr << err << endl;
		return;
	}
	//формируем файл логов:
	ofstream flog;
	flog.open(".\\logfile.log", ios_base::out | ios_base::trunc);
	//проходим по списку файлов
	for (auto a : arr)
	{
		fs::path p(a.first + "\\" + a.second);
		Copy4File(p, src, flog);
	}
	flog.close();
}