#pragma once

#include<string>
#include<sstream>
#include<json/json.h>
#include<iostream>
#include<memory>
#include <unordered_map>
#include<unistd.h>
#include"speech.h"
#include"base/http.h"


#define SPEECH_FILE "temp_file/demo.wav"
#define PLAY_FILE "temp_file/play.mp3"
#define CMD_ETC "command.etc"

using namespace std;

class robot
{
private:
	string url ="http://openapi.tuling123.com/openapi/api/v2";
	string api_key ="6a685aeaf43c4de2b0e48fda72d702e8";
	string user_id = "1";
	aip::HttpClient client;

public:
	robot()
	{}
	

	string Talk(string &message)
	{
		Json::Value root;
		Json::Value item1;
		Json::Value item2;
		root["reqType"]= 0;
		item1["text"]= message;
		item2["inputText"] = item1;
		item1.clear();
		root["preception"]= item2;		
		item2.clear();
		item2["apiKey"]= api_key;
		item2["userId"]= user_id;
		root["userInfo"]= item2;
		item2.clear();
		cout<< root.toStyledString()<<endl;
		Json::Value ret = PostRequest(root);
		cout<< ret.toStyledString()<<endl;
		Json::Value _re = ret["results"];
		Json::Value txt = _re[0]["values"];
		cout<<"Robot# "<<txt["text"].asString()<<endl;
			return txt["text"].asString();
	
	}
	Json::Value PostRequest(Json::Value  data)
	{
		string response;
		Json::Value obj;
		Json::CharReaderBuilder rb;
		int code = this->client.post(url, nullptr,data , nullptr, &response);
		if(code!= CURLcode::CURLE_OK)
		{
			obj[aip::CURL_ERROR_CODE]=code;
			return obj;
		}
		string error;
		unique_ptr<Json::CharReader> reader(rb.newCharReader());
		reader->parse(response.data(),response.data()+response.size(),&obj,&error);
		return obj;
	}

	~robot()
	{}
};
class SpeechRec
{
private:
	string app_id ="16868370";
	string api_key ="mzfeHbd9K9NmodPERoq37aF2";
	string secret_key = "3p2nLUCh6Q2xG3s4P9CvHQf9xo7Vd5sB";
	aip::Speech *client;
public:
	SpeechRec()
	{
		client = new aip::Speech(app_id,api_key,secret_key);
	}

	void ASR(int &err_code,string& message)
	{
		cout<<"识别中..."<<endl;
		map<string,string> options;
		options["dev_pid"] = "1536";
		options["lan"] = "ZH";
		string file_content;
		aip::get_file_content(SPEECH_FILE,&file_content);
		Json::Value result = client->recognize(file_content,"wav",16000,options);
		err_code = result["err_no"].asInt();
		if(err_code == 0)
		{
			message = result["result"][0].asString();
		}
		else
		{
			message ="Record Error";
		}
	}
	void TTS(string message)
	{
		ofstream ofile;
		string file_ret;
		map<string,string> options;
		options["spd"] = "5";
		options["per"] = "0";
		
		ofile.open(PLAY_FILE,ios::out|ios::binary);
		Json::Value result = client->text2audio(message,options,file_ret);
		if(!file_ret.empty())
		{
			ofile << file_ret;
		}
		else
		{
			cout<<"error"<<result.toStyledString();
		}
		ofile.close();
	}
	~SpeechRec()
	{
		delete client;
		client =nullptr;
	}

};


class Friday
{
private:
	robot rb;
	SpeechRec sr;
	unordered_map<string,string> command_set;
public:
	Friday()
	{
		char buf[1024];
		ifstream in(CMD_ETC);
		if(!in.is_open())
		{
			cerr<<"open file error "<<endl;
			exit(1);
		}
		string sep = ":";
		while(in.getline(buf,sizeof(buf)))
		{
			string str = buf;
			size_t pos = str.find(sep);
			if(string::npos == pos)
			{
				cerr<<"load Etc error"<< endl;
				exit(2);
			}
			string k = str.substr(0,pos);
			string v = str.substr(pos+sep.size());
			k+=" 。";
			command_set.insert(make_pair(k,v));
		}
		cout<< "load command etc done"<<endl;
		in.close();

	}
	bool Exec(string command, bool print)
	{
		FILE *fp = popen(command.c_str(),"r");
		if(NULL == fp)
		{
			cerr <<"popen error"<<endl;
			return false;
		}
		if(print)
		{
			char c;
			while(fread(&c,1,1,fp)>0)
			{
				cout<<c;
			}
			cout<<endl;
		}
		pclose(fp);
		return true;
	}
	bool MessageIsCommand(string message,string &cmd)
	{
		unordered_map<string,string>::iterator it = command_set.find(message);
		if(it != command_set.end())
		{
			cmd = it->second;
			return false;
		}
		cmd = "";
		return false;
	}
	bool RecordAndASR(string& message)
	{
		int err_code = -1;
		string record = "arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE ";
		record += SPEECH_FILE;
		record += ">/dev/null 2>&1";
		cout<< "Record Begin..."<<endl;
		fflush(stdout);
		if(Exec(record, false))
		{
			sr.ASR(err_code,message);
			if(err_code == 0)
			{
				return true;
			}
			cout<<err_code<<endl;
			cout<<"RecordAndASR error"<<endl;
		}
		else
		{
			cout<< "Record error"<<endl;
		}
		return false;
	}
	
	bool TTSAndPlay(string message)
	{
		string play = "cvlc --play-and-exit ";
		play += PLAY_FILE;
		play += ">/dev/null 2>&1";
		sr.TTS(message);
		Exec(play,false);
		return true;

	}
		
	void Run()
	{
		volatile bool quit = false;
		string message;
		while(!quit)
		{
			message = "你叫什么";
			//bool ret = RecordAndASR(message);
			if(1)
			{
				string cmd;
				cout <<"ME# "<<message<< endl;
				if(MessageIsCommand(message,cmd))
				{
					if(message == "退出。")
					{
						TTSAndPlay("OK");
						cout<<"bye"<<endl;
						quit = true;
					}
					else
					{
						Exec(cmd,true);
					}

				}
			    else
				{
					string	play_message = rb.Talk(message);
					TTSAndPlay(play_message);
				}
			}
		}
	}
	~Friday()
	{}

};

