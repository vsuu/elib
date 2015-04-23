#include "stdafx.h"
#include "CfgBase.h"
#include <exception>
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using namespace __LIB_NAME__;

void CfgTest()
{
    system("del cfg_test.xml");
    system("echo 开始测试：");
    system("echo ^<root^>^</root^>  >cfg_test.xml");

    try
    {
        {//add
            XMLCfgFile cfg;
            cfg.Load("cfg_test.xml");
            cfg.Add("c_string_test/single", "renfeiting");
            cfg.Add("c_string_test/list", "abc");
            cfg.Add("c_string_test/list", "efg");
            cfg.Add("int_test/single", 3);
            cfg.Add("int_test/list", 4);
            cfg.Add("int_test/list", 5);
            cfg.Add("double_test/single", 3.4);
            cfg.Add("double_test/list", 4.4);
            cfg.Add("double_test/list", 5.5);
            cfg.Add("bool_test/single", true);
            cfg.Add("bool_test/list", false);
            cfg.Save();
        }
        {//get
            XMLCfgFile cfg;
            cfg.Load("cfg_test.xml");
            if (3 != cfg.Get<int>("int_test/single"))
            {
                throw "int_test single fail!";
            }
            auto tmp = cfg.GetList<int>("int_test/list");
            if (tmp != vector < int > {4, 5})
            {
                throw "int_test/list fail!";
            }
            if (string("renfeiting") != cfg.Get<const char *>("c_string_test/single"))
            {
                throw "c_string_test single fail";
            }

            auto cstringtmp = cfg.GetList<const char *>("c_string_test/list");
            if (vector < string > {"abc", "efg"} != vector<string>(begin(cstringtmp), end(cstringtmp)))
            {
                throw "c_string_test list fail";
            }
        }
        {//set
            XMLCfgFile cfg;
            cfg.Load("cfg_test.xml");
            cfg.Set("double_test/single", 3.5);
            cfg.Set("double_test/list", 4.4, 4.5);
            cfg.Set("int_test/single", 30);
            if (cfg.Get<int>("int_test/single") != 30)
            {
                throw "int_test single set fail";
            }
            if (cfg.Get<double>("double_test/single") != 3.5)
            {
                throw "double_test single set fail";
            }
            if (cfg.GetList<double>("double_test/list") != vector < double > {4.5, 5.5})
            {
                throw "double_test list set fail";
            }
            cfg.Save();
        }
        {//del
            XMLCfgFile cfg;
            cfg.Load("cfg_test.xml");
            cfg.Del("double_test/list", 4.5);

            if (cfg.GetList<double>("double_test/list").size() != 1)
            {
                throw "Del fail!";
            }
            cfg.Save();
        }
        system("echo 测试通过!");
    }
    catch (const exception &e)
    {
        cout << "error :";
        cout << e.what() << endl;
    }
    catch (const char *e)
    {
        cout << "error :";
        cout << e << endl;
    }
}