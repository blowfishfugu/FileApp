/**
 \file
 \brief   file with the implementation of independent parts of the process logic for the FileApp program
 \details This file contain the independent implementation of the processes in this application.
          The core is the class TProcess.
 \details By the use of the header-only library adecc Scholar the independence of the respective framework
          is achieved.
 <hr>
 \author Volker Hillmann (adecc Systemhaus GmbH)
 \date 22.07.2022 Create repository with this file
 \version 0.1
 \since Version 0.1
*/
#pragma hdrstop

#include "Process.h"

#include "MyForm.h"
#include "MyStream.h"
#include "MyType_Traits.h"
#include "MyTupleUtils.h"

#include "FileUtil.h"

#include "pugixml.hpp"


#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include <functional>
#include <exception>
#include <fstream>

//---------------------------------------------------------------------------
#pragma package(smart_init)

// --------------------------------------------------------------------------
// Prozesslogik

struct TMyNum : public std::numpunct<char> {
	char_type   do_decimal_point () const { return ','; }
	char_type   do_thousands_sep () const { return '.'; }
	string_type do_truename ()      const { return "ja"; }
	string_type do_falsename ()     const { return "nein"; }
	std::string do_grouping ()      const { return "\3";	}
   };

TMyNum newNumPunct;
std::locale TProcess::myLoc;

TStreamWrapper<Narrow> old_cout(std::cout);
TStreamWrapper<Narrow> old_cerr(std::cerr);
TStreamWrapper<Narrow> old_clog(std::clog);


/// vector with captions and params for the project list
std::vector<tplList<Narrow>> TProcess::Project_Columns {
    		  tplList<Narrow> { "project",      360, EMyAlignmentType::left },
              tplList<Narrow> { "path",         650, EMyAlignmentType::left },
              tplList<Narrow> { "type",         150, EMyAlignmentType::left },
			  tplList<Narrow> { "order",        110, EMyAlignmentType::right },
    		  tplList<Narrow> { "cpp- file",    350, EMyAlignmentType::left },
    		  tplList<Narrow> { "rows",         110, EMyAlignmentType::right },
			  tplList<Narrow> { "h- file",      280, EMyAlignmentType::left },
			  tplList<Narrow> { "rows",         110, EMyAlignmentType::right },
			  tplList<Narrow> { "form file",   350, EMyAlignmentType::left },
			  tplList<Narrow> { "form name",    340, EMyAlignmentType::left },
			  tplList<Narrow> { "rows",         110, EMyAlignmentType::right },
			  tplList<Narrow> { "formType",     150, EMyAlignmentType::left },
			  tplList<Narrow> { "designclass",  250, EMyAlignmentType::left } };

/// vector with captions and params for counting files
std::vector<tplList<Narrow>> TProcess::Count_Columns {
    		  tplList<Narrow> { "files",        450, EMyAlignmentType::right },
              tplList<Narrow> { "directories",  450, EMyAlignmentType::right },
              tplList<Narrow> { "size",         600, EMyAlignmentType::right }
              };

/// vector with captions and params for the file list
std::vector<tplList<Narrow>> TProcess::File_Columns {
    		  tplList<Narrow> { "file",       1310, EMyAlignmentType::left },
              tplList<Narrow> { "time",        265, EMyAlignmentType::left },
              tplList<Narrow> { "size",        150, EMyAlignmentType::right } };


constexpr int iMyData_Project  =  0; ///< constant for position of name of project in tplData
constexpr int iMyData_Path     =  1; ///< constant for position of path to project in tplData
constexpr int iMyData_Type     =  2; ///< constant for type of node in tplData 
constexpr int iMyData_Order    =  3; ///< constant for build order in tplData
constexpr int iMyData_CppFile  =  4; ///< constant for source file with relative path in tplData
constexpr int iMyData_CppRows  =  5; ///< constant for rows in source file in tplData
constexpr int iMyData_H_File   =  6; ///< constant for name with relative path of dependent header file
constexpr int iMyData_H_Rows   =  7; ///< constant for rows in header file in tplData
constexpr int iMyData_FrmFile  =  8; ///< constant for name with relative path of dependent form file in tplData
constexpr int iMyData_FrmName  =  9; ///< constant for name of the formular in dependent form file in tplData
constexpr int iMyData_FrmRows  = 10; ///< constant for rows in form file in tplData
constexpr int iMyData_FrmType  = 11; ///< constant for type of form in dependent form file in tplData 
constexpr int iMyData_FrmClass = 12; ///< constant for used design class in tplData


/**
  \brief tuple with all Data for projects in cbproj- files
  \details list of parts
<table><tr><th> ID <th> Constant <th> Type <th> Description</tr>
<tr><td align="right"><b>  0 </b> <td align="left"> iMyData_Project  <td>std::string <td>name of the project file </tr>
<tr><td align="right"><b>  1 </b> <td align="left"> iMyData_Path     <td>std::string <td>path to the project file </tr>
<tr><td align="right"><b>  2 </b> <td align="left"> iMyData_Type     <td>std::string <td>type of node </tr>
<tr><td align="right"><b>  3 </b> <td align="left"> iMyData_Order    <td>int         <td>build order </tr>
<tr><td align="right"><b>  4 </b> <td align="left"> iMyData_CppFile  <td>std::string <td>source file with relative path </tr>
<tr><td align="right"><b>  5 </b> <td align="left"> iMyData_CppRows  <td>size_t      <td>rows in source file </tr>
<tr><td align="right"><b>  6 </b> <td align="left"> iMyData_H_File   <td>std::string <td>name of header file </tr>
<tr><td align="right"><b>  7 </b> <td align="left"> iMyData_H_Rows   <td>size_t      <td>rows in header file </tr>
<tr><td align="right"><b>  8 </b> <td align="left"> iMyData_FrmFile  <td>std::string <td>form file </tr>
<tr><td align="right"><b>  9 </b> <td align="left"> iMyData_FrmName  <td>std::string <td>name of the formular </tr>
<tr><td align="right"><b> 10 </b> <td align="left"> iMyData_FrmRows  <td>size_t      <td>rows in form file </tr>
<tr><td align="right"><b> 11 </b> <td align="left"> iMyData_FrmType  <td>std::string <td>formtype </tr>
<tr><td align="right"><b> 12 </b> <td align="left"> iMyData_FrmClass <td>std::string <td>design class </tr> </table>
*/
using tplData = std::tuple<std::string,  //  0 project
                           std::string,  //  1 path
                           std::string,  //  2 type
                           int,          //  3 build order
                           std::string,  //  4 cpp file
                           size_t,       //  5 rows
                           std::string,  //  6 h file
                           size_t,       //  7 rows
                           std::string,  //  8 form file
                           std::string,  //  9 form name
                           size_t,       // 10 rows
                           std::string,  // 11 formtype
                           std::string>; // 12 design class


//----------------------------------------------------------------------------
template <typename Iterator, typename Pred, typename Operation>
void for_each_if(Iterator begin, Iterator end, Pred pred, Operation op) {
   if(begin < end) {
      for(; begin != end; ++begin) {
         if(pred(*begin)) op(*begin);
         }
      }
   }


template <typename string_type, typename container>
size_t parse(string_type const& source, std::string const& del, container& list) {
   static_assert(is_vector<container>::value || is_set<container>::value, "unexpected value for container in function parse");
   size_t pos = 0u, end;
   do {
       end = source.find(del, pos);
       string_type part = source.substr(pos, end - pos);
       if(auto start_at = part.find_first_not_of(" \t"); start_at != string_type::npos) {
          auto end_at   = part.find_last_not_of(" \t");
          string_type trimmedpart = part.substr(start_at, end_at - start_at + 1);
          if(trimmedpart.length() > 0) {
             if constexpr (is_vector<container>::value) {
                list.emplace_back(trimmedpart);
                }
             else if constexpr (is_set<container>::value) {
                list.insert(trimmedpart);
                }
             else {
                static_assert_no_match();
                }
             }
          }
      pos = end + del.length();
      } while (end != string_type::npos);
   return list.size();
   }





 /**
   \brief Intitialize the main window of the application
   \param form [IN] rvalue to form, which is used as main window
   \throw std::runtime_error
*/
 void TProcess::Init(TMyForm&& form) {
   swap(frm, form);
   std::ios_base::sync_with_stdio(false);
   myLoc = std::locale(std::locale("de_DE"), &newNumPunct);

   frm.GetAsStream<Narrow, EMyFrameworkType::listview>(old_cout, "lvOutput", Project_Columns);
   frm.GetAsStream<Narrow, EMyFrameworkType::memo>(old_cerr, "memError");
   frm.GetAsStream<Narrow, EMyFrameworkType::statusbar>(old_clog, "sbMain");

   for(auto stream : { &std::cout, &std::cerr, &std::clog } ) {
      stream->imbue(myLoc);
      stream->setf(std::ios::fixed);
      }

   frm.SetCaption("analyzing projects");

   frm.Set<EMyFrameworkType::label>("lblDirectory", "directory:");
   frm.Set<EMyFrameworkType::edit>("edtDirectory", "d:\\projekte\\vorlesung");

   frm.Set<EMyFrameworkType::button>("btnCount", "count");
   frm.Set<EMyFrameworkType::button>("btnShow",  "show");     // !!!
   frm.Set<EMyFrameworkType::button>("btnParse", "parse");

   std::ostream mys(frm.GetAsStreamBuff<Narrow, EMyFrameworkType::listbox>("lbValues"));
   std::vector<std::string> test = { ".cpp", ".h", ".dfm", ".fmx", ".cbproj", ".c", ".hpp" };
   std::copy(test.begin(), test.end(), std::ostream_iterator<std::string>(mys, "\n"));

   std::clog << "program ready for action" << std::endl;

   tasks.start();
   tasks.add_task(100, [this]() { static int i = 0;
                                  std::ostringstream os;
                                  os << ++i << ". Test";
                                  this->frm.Set<EMyFrameworkType::edit>("edtTest",  os.str());
                                  return true;
                                });

   }



void TProcess::ShowAction() {
   try {
      std::vector<fs::path> files;
      //std::set<std::string> extensions = { ".cpp", ".c", ".h", ".hpp", ".hxx", ".cbproj", ".dfm", ".fmx" };
      std::set<std::string> extensions;
      my_formlist<EMyFrameworkType::listbox, std::string> mylist(&frm, "lbValues");
      std::copy(mylist.begin(), mylist.end(), std::ostream_iterator<std::string>(std::cerr, "\n"));
      std::copy(mylist.begin(), mylist.end(), std::inserter(extensions, extensions.end()));

      auto strPath = frm.Get<EMyFrameworkType::edit, std::string>("edtDirectory");
      if(!strPath) {
         throw std::runtime_error("directory to show is empty, set a directory before call this function");
         }
      else {
         frm.GetAsStream<Narrow, EMyFrameworkType::listview>(old_cout, "lvOutput", File_Columns);
         std::chrono::milliseconds time;
         fs::path fsPath = *strPath;
         auto ret = Call(time, Find, std::ref(files), std::cref(fsPath), std::cref(extensions), true);
         std::clog << "function \"Find\" procecced in "
                   << std::setprecision(3) << time.count()/1000. << " sec, "
                   << files.size() << " files found" << std::endl;
         ShowFiles(std::cout, files);
         }
      }
   catch(std::exception& ex) {
      std::cerr << "error in function \"Show\": " << ex.what() << std::endl;
      std::clog << "error in function \"Show\"" << std::endl;
      }
   tasks.stop();
   }

/** \brief construction of filename with informations from tplData and base directory
\tparam iFile Contant of int with the position of relative name in tplData
\param base const reference of fs::path with basic path for tplData
\param row const reference of tplData with informations for a information in project file
\return fs::path with the absolute path to the requested file
*/
template <int iFile>
fs::path ConstructFile(fs::path const& base, tplData const& row) {
   return fs::weakly_canonical(base / fs::path(std::get<iMyData_Path>(row)) / fs::path(std::get<iFile>(row)));
}

/// method to parse a cbproj file for informations
void Parse(fs::path const& base, fs::path const& strFile, std::vector<tplData>& projects) {

   try {
      pugi::xml_document doc;
      pugi::xml_parse_result result = doc.load_file(strFile.string().c_str(), pugi::parse_default | pugi::parse_fragment);
      if(result) {
         pugi::xml_node root = doc.document_element();
         pugi::xpath_node xpathNode = root.select_single_node("ItemGroup");
         if(xpathNode) {
            pugi::xml_node selNode = xpathNode.node();
            for(pugi::xml_node child = selNode.child("CppCompile"); child; child = child.next_sibling("CppCompile")) {
               tplData row;
               std::get<iMyData_Project>(row)  = strFile.filename().string();
               std::get<iMyData_Path>(row)     = fs::relative(strFile.parent_path(), base).string();
               std::get<iMyData_Type>(row)     = "Cpp Node";
               std::get<iMyData_Order>(row)    = atoi(child.child_value("BuildOrder"));
               std::get<iMyData_CppFile>(row)  = child.attribute("Include").value();
               std::get<iMyData_H_File>(row)   = child.child_value("DependentOn");
               std::get<iMyData_FrmName>(row)  = child.child_value("Form");
               std::get<iMyData_FrmType>(row)  = child.child_value("FormType");
               std::get<iMyData_FrmClass>(row) = child.child_value("DesignClass");

               if(!std::get<iMyData_CppFile>(row).empty())
                  //std::get<iMyData_CppRows>(row) =CheckFileSize(fs::weakly_canonical(base / fs::path(std::get<iMyData_Path>(row)) / fs::path(std::get<iMyData_CppFile>(row))));
                  std::get<iMyData_CppRows>(row) =CheckFileSize(ConstructFile<iMyData_CppFile>(base, row));

               if(!std::get<iMyData_H_File>(row).empty())
                  //std::get<iMyData_H_Rows>(row) =CheckFileSize(fs::weakly_canonical(base / fs::path(std::get<iMyData_Path>(row)) / fs::path(std::get<iMyData_H_File>(row))));
                  std::get<iMyData_H_Rows>(row) =CheckFileSize(ConstructFile<iMyData_H_File>(base, row));


               if(!std::get<iMyData_FrmName>(row).empty()) {
                  std::string strExt;
                  if(std::get<iMyData_FrmType>(row).empty()) strExt = ".dfm";   // eventual setting
                  else strExt = std::string(".") + std::get<iMyData_FrmType>(row);
                  std::get<iMyData_FrmFile>(row) = ( fs::path(std::get<iMyData_CppFile>(row)).parent_path() /
                                                     fs::path(std::get<iMyData_CppFile>(row)).stem()).string() +
                                                     strExt;
                  //std::get<iMyData_FrmRows>(row) =CheckFileSize(fs::weakly_canonical(base / fs::path(std::get<iMyData_Path>(row)) / fs::path(std::get<iMyData_FrmFile>(row))));
                  std::get<iMyData_FrmRows>(row) =CheckFileSize(ConstructFile<iMyData_FrmFile>(base, row));
                  }
               projects.emplace_back(std::move(row));
               }
            }
         }
      }
   catch(std::exception& ex) {
      std::cerr << ex.what() << std::endl;
      }
   }


void TProcess::ParseAction() {
   std::vector<fs::path> project_files;
   std::vector<tplData> projects;
   std::set<std::string> extensions = { ".cbproj" };

   try {
      auto strPath = frm.Get<EMyFrameworkType::edit, std::string>("edtDirectory");
      if(!strPath) {
         throw std::runtime_error("directory to parse is empty, set a directory before call this function");
         }
      else {
         frm.GetAsStream<Narrow, EMyFrameworkType::listview>(old_cout, "lvOutput", Project_Columns);
         std::chrono::milliseconds time;
         fs::path fsPath = *strPath;
         auto ret = Call(time, Find, std::ref(project_files), std::cref(fsPath), std::cref(extensions), true);
         std::clog << ret << " files found, "
                   << "procecced in " << std::setprecision(3) << time.count()/1000. << " sec" << std::endl;


         for(auto file : project_files) Parse(fsPath, file, projects);

         std::sort(projects.begin(), projects.end(), [](auto lhs, auto rhs) {
                      if(auto ret = std::get<iMyData_Project>(lhs).compare(std::get<iMyData_Project>(rhs)); ret == 0) {
                         if(auto ret = std::get<iMyData_Path>(lhs).compare(std::get<iMyData_Path>(rhs)); ret == 0) {
                            return std::get<iMyData_Order>(lhs) < std::get<iMyData_Order>(rhs);
                            }
                         else return ret < 0;
                         }
                      else return ret < 0;
                      });

         TMyDelimiter<Narrow> delimiter = { "", "\t", "\n" };
         std::for_each(projects.begin(), projects.end(), [&delimiter](auto val) {
            myTupleHlp<Narrow>::Output(std::cout, delimiter, val);
            });
         }
      }
   catch(std::exception &ex) {
      std::cerr << "error in function \"Parse\": " << ex.what() << std::endl;
      std::clog << "error in function \"Parse\"" << std::endl;
      }
   }

void TProcess::CountAction() {
   try {
      auto strPath = frm.Get<EMyFrameworkType::edit, std::string>("edtDirectory");
      if(!strPath) {
         throw std::runtime_error("directory to count is empty, set a directory before call this function");
         }
      else {
         frm.GetAsStream<Narrow, EMyFrameworkType::listview>(old_cout, "lvOutput", Count_Columns);
         std::chrono::milliseconds time;
         fs::path fsPath = *strPath;
         auto ret = Call(time, Count, std::cref(fsPath), true);
         std::get<2>(ret) = Convert_Size_KiloByte(std::get<2>(ret));
         TMyDelimiter<Narrow> delimiter = { "", "\t", "\n" };
         myTupleHlp<Narrow>::Output(std::cout, delimiter, ret);
         std::clog << "function \"Count\" procecced in "
                   << std::setprecision(3) << time.count()/1000. << " sec" << std::endl;
         }
      }
   catch(std::exception &ex) {
      std::cerr << "error in function \"Count\": " << ex.what() << std::endl;
      std::clog << "error in function \"Count\"" << std::endl;
      }
   }
