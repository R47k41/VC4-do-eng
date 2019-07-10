#include <sstream>
#include <cctype>
#include <stdexcept>
#include <Windows.h>
#include <vector>
#include <map>
#include <math.h>
#include <boost/filesystem.hpp>
#include "UsingClasses.h"
//#include <boost/algorithm/string.hpp>
namespace fs = boost::filesystem;

using std::multimap;
using std::exception;
using std::cout;
using std::ostream;
using std::replace;
using fs::file_size;
using fs::exists;
using fs::is_directory;
using fs::path;
using fs::directory_iterator;
using fs::directory_entry;
using fs::filesystem_error;

const int PRCNT_STEP = 20;

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

//установка локали
inline void SetRusLang(int cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); };

//***Источник - eng версия
//***Приемник - jpn версия

//**** Основная суть программы заменить файлы в японской версии на
//файлы английской версии


//1. устанавливаем путь к источнику и путь к приемнику - это должны быть папки
fs::path GetUsrPath(const string& msg) noexcept(false);

//2. получение списка файлов в приемнике: путь + имя
TFileArr getFilesArraybyPath(const fs::path& p);

//3. Для каждого файла из источника ищем его в приемнике + запись в логе
//поиск файла в папке источнике
const TFileArrIterator& find_file(const TFileArrIterator::value_type& f_dst, const TFileArr& dir, TLogger& log) noexcept(true);
//копирование для одного файла
void copy4file(const TFileArrIterator::value_type& f_dst, const TFileArrIterator& f_src, TLogger& log) noexcept(true);
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
}

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
}

//поиск файла в папке источнике
const TFileArrIterator& find_file(const TFileArrIterator::value_type& f_dst, const TFileArr& dir, TLogger& log) noexcept(true)
{
	using std::stringstream;
	
	stringstream msg;
	//cout << "Поиск файла: " << a.first << endl;
	if (dir.count(f_dst.first) > 1)
	{
		msg << "****************************************" << endl;
		msg << "Для файла " << f_dst.first << " найдено несколько совпадений:" << endl;
		TFilesArrRange range = dir.equal_range(f_dst.first);
		int tmp = 0;
		for (TFileArrIterator x = range.first; x != range.second; x++)
			msg << ++tmp << ": " << x->second << "\\" << x->first << endl;
		msg << "****************************************" << endl;
		cout << msg.str();
		log.Add2Many_log(msg.str());
		return dir.end();
	}
	TFileArrIterator itr = dir.find(f_dst.first);
	if (itr == dir.end())
	{
		msg << "Файл " << f_dst.second << "\\" << f_dst.first << " не найден!" << endl;
		cout << msg.str();
		log.Add2NoFnd_log(msg.str());
		return dir.end();
	}
	else
		return itr;
	return dir.end();
}

//копирование для одного файла
void copy4file(const TFileArrIterator::value_type& f_dst, const TFileArrIterator& f_src, TLogger& log) noexcept(true)
{
	using std::stringstream;
	stringstream msg;
	//формируем файл который бедем заменять		
	fs::path old_file(f_dst.second + '\\' + f_dst.first);
	//формируем файл на который будем заменять
	fs::path new_file(f_src->second + '\\' + f_src->first);
	uintmax_t old_sz = file_size(old_file);
	uintmax_t new_sz = file_size(new_file);
	int percent_sz = (old_sz == 0 ? 100 : (old_sz - new_sz) / old_sz);
	if (std::abs(percent_sz) > PRCNT_STEP)
		msg << "***WARNING***\t";
	msg << "Файл: " << old_file.filename().string() << "(" << old_sz;
	msg << "/" << new_sz << ") заменен файлом из каталога: " << new_file.filename().string();
	try
	{
		fs::copy_file(new_file, old_file, fs::copy_option::overwrite_if_exists);
	}
	catch (fs::filesystem_error err)
	{
		log.Add2Rpl_log(err.what());
	}
	log.Add2Rpl_log(msg.str());
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
	TLogger log;
	//получаем список файлов источника:
	cout << "Формируем список файлов источника: " << src.string() << endl;
	TFileArr SrcFiles = getFilesArraybyPath(src);
	cout << "Сортируем список файлов источника" << endl;
	//проходим по списку файлов
	long cnt = 0;
	long all_cnt = arr.size();
	for (auto a : arr)
	{
		const TFileArrIterator& itr = find_file(a, SrcFiles, log);
		if (itr != SrcFiles.end()) copy4file(a, itr, log);
		cout << "Обработано " << cnt++ << " файлов из " << all_cnt << endl;
	}
}