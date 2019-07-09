#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <Windows.h>
#include <vector>
#include <map>
#include <math.h>
#include <algorithm>
#include <iterator>
#include <utility>
#include <boost/filesystem.hpp>
//#include <boost/algorithm/string.hpp>
namespace fs = boost::filesystem;

using std::string;
using std::multimap;
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
typedef std::multimap<std::string, std::string, std::less<string> > TFileArr;
//итератор для списка файлов
typedef TFileArr::const_iterator TFileArrIterator;
//диапазон
typedef std::pair<TFileArrIterator, TFileArrIterator> TFilesArrRange;

template <class T = string>
struct less
{
	bool operator()(const T& a, const T& b) const;
};

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
	less<string> l;
	cout << l("Hello", "hello") << endl;
	return 0;
	try
	{
		//получение директории файлов приемника
		//path SrcDir = GetUsrPath("Укажите путь к источнику: ");
		path SrcDir("J:\\Temp\\RUS_make\\VC4\\PC\\original");
		if (SrcDir.filename().empty()) return 0;
		//path DestDir = GetUsrPath("Укажите путь к приемнику: ");
		path DestDir("J:\\Temp\\RUS_make\\VC4\\PS4\\Source");
		if (DestDir.filename().empty()) return 0;
		TFileArr DestFiles;
		DestFiles = getFilesArraybyPath(DestDir);
		//for (auto a : DestFiles)
			//cout << a.first << " в папке: " << a.second << endl;
		cout << "Начинаем процедуру копировани..." << endl;
		CopyFiles(DestFiles, SrcDir);
		cout << "Процедура копирования завершена." << endl;
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

template <>
bool less<std::string>::operator()(const std::string& a, const std::string& b) const
{
	string f1 = set_chr_case(a, std::tolower);
	string f2 = set_chr_case(b, std::tolower);
	return f1 < f2;
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
	//using std::vector;
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
	//vector<TFileRec> FilesLst;
	for (directory_entry dir : directory_iterator(p))
	{
		//если это директория
		if (is_directory(dir))
		{
			TFileArr tmpArr;
			tmpArr = getFilesArraybyPath(dir.path());
			cout << "Получаем список файлов для директории: " << dir.path() << endl;
			result.insert(tmpArr.begin(), tmpArr.end());
		}
		else
		{
			TFileRec tmpFile;
			tmpFile.first = dir.path().filename().string();
			tmpFile.second = dir.path().parent_path().string();
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
	return set_chr_case(file1, fnc) < set_chr_case(file2, fnc) ? true : false;
}

//поиск файла в папке источнике
fs::path getFileInSrc(const string& fname, const fs::path& dir) noexcept(false)
{
	cout << "Поиск файла: " << fname << " в директории: " << dir.string() << endl;
	for (directory_entry f : directory_iterator(dir))
	{
		cout << "Поиск в " << f.path().string() << endl;
		if (is_directory(f))
		{
			
			fs::path p = getFileInSrc(fname, f.path());
			//если файл найден - выходим
			if (!p.empty()) 
				return p;
			else
				continue;
		}
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
	if (is_directory(pfile))
	{
		log << "Для поиска указана директория: " << pfile.string() << endl;
		cout << "Для поиска указана директория: " << pfile.string() << endl;
		return;
	}
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
		/*
		try
		{
			fs::copy_file(p, pfile, fs::copy_option::overwrite_if_exists);
		}
		catch (fs::filesystem_error err)
		{
			log << err.what() << endl;
		}
		/**/
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
	using std::sort;
	using std::stringstream;

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
	//получаем список файлов источника:
	cout << "Формируем список файлов источника: " << src.string() << endl;
	TFileArr SrcFiles = getFilesArraybyPath(src);
	cout << "Сортируем список файлов источника" << endl;
	stringstream msg;
	//проходим по списку файлов
	long cnt = 0;
	long all_cnt = arr.size();
	for (auto a : arr)
	{
		msg.clear();
		//cout << "Поиск файла: " << a.first << endl;
		if (SrcFiles.count(a.first) > 1)
		{
			msg << "****************************************" << endl;
			msg << "Для файла " << a.first << " найдено несколько совпадений!!!" << endl;
			TFilesArrRange range = SrcFiles.equal_range(a.first);
			int tmp = 0;
			for (TFileArrIterator x = range.first; x != range.second; x++)
				msg << ++tmp << ": " << x->second << "\\" << x->first << endl;
			msg << "****************************************" << endl;
			cout << msg.str();
			flog << msg.str();
			continue;
		}
		TFileArr::const_iterator itr = SrcFiles.find(a.first);
		if (itr == SrcFiles.end())
		{
			msg << "Файл " << a.second << "\\" << a.first << " не найден!"<< endl;
			cout << msg.str();
			flog << msg.str();
			continue;
		}
		else
		{
			fs::path fold(a.second + '\\' + a.first);
			fs::path fnew(itr->second + '\\' + itr->first);
			uintmax_t old_sz = file_size(fold);
			uintmax_t new_sz = file_size(fnew);
			int percent_sz = (old_sz == 0 ? 100 : (old_sz - new_sz) / old_sz);
			if (std::abs(percent_sz) > 35)
				msg << "***WARNING***\t";
			msg << "Файл: " << fold.filename().string() << "(" << old_sz;
			msg << "/" << new_sz << ") заменен файлом из каталога: " << fnew.filename().string();
			flog << msg.str();
		}
		cout << "Обработано " << cnt++ << " файлов из " << all_cnt << endl;
	}
	flog.close();
}