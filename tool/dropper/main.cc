#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include "split.h"
//https://www.cnblogs.com/MikeZhang/archive/2012/03/24/MySplitFunCPP.html
//char *strtok(char *str, const char *delim);
using namespace std;
enum NaluType{
SPS,
PPS,
SEI,
ISLICE,
PSLICE,
BSLICE,
SISLICE,
SPSLICE,
NONENALU,
};
const char* TypeToString(int type){
    switch(type){
        case SPS:{
            return "SPS";
        }
        case PPS:{
            return "PPS";
        }
        case SEI:{
            return "SEI";
        }
        case ISLICE:{
            return "ISlice";
        }
        case PSLICE:{
            return "PSlice";
        }
        case BSLICE:{
            return "BSlice";
        }
        case SISLICE:{
            return "SISlice";
        }
        case SPSLICE:{
            return "SPSlice";
        }
        default:
            return "NONENALU";
    }
}
int GetType(std::string &s){
    if(s==TypeToString(SPS)){
        return SPS;
    }
    if(s==TypeToString(PPS)){
        return PPS;
    }
    if(s==TypeToString(SEI)){
        return SEI;
    }
    if(s==TypeToString(ISLICE)){
        return ISLICE;
    }
    if(s==TypeToString(PSLICE)){
        return PSLICE;
    }
    if(s==TypeToString(BSLICE)){
        return BSLICE;
    }
    if(s==TypeToString(SISLICE)){
        return SISLICE;
    }
    if(s==TypeToString(SPSLICE)){
        return SPSLICE;
    }
    return NONENALU;
}
struct NaluLogInfo{
NaluLogInfo(int a1,int a2,int a3,int a4):
no(a1),start(a2),offset(a3),type(a4){}
int no;
int start;
int offset;
int type;
};
int drop_table[]={6,7,8,9};
bool should_drop(int no,int *droptable,int len){
	bool ret=false;
	int i=0;
	for(i=0;i<len;i++){
		if(droptable[i]==no){
			ret=true;
			break;
		}
	}
	return ret;
}
class H264BinInfo{
public:
    void OnNewNaluInfo(NaluLogInfo info){
        infos_.insert(std::make_pair(info.no,info));
    }
    int Size() const{
        return infos_.size();
    }
    const std::map<int,NaluLogInfo>& GetInfo(){
    	return infos_;
    }
private:
    std::map<int,NaluLogInfo> infos_;
};
class ReadBuffer{
public:
	void Reset(int size){
		if(size>len_){
			buf_.reset(new char[size]);
			len_=size;
		}
	}
	char *data(){
		return (char*)buf_.get();
	}
	int Len() const{
		return len_;
	}
private:
	std::unique_ptr<char []> buf_;
	int len_{0};
};
class FrameDroper{
public:
	FrameDroper(std::string &in,std::string &out){
		f_in_.open(in.c_str(),ios::in|ios::binary);
		f_out_.open(out.c_str(),ios::out|ios::binary);
	}
	~FrameDroper(){
		if(f_in_.is_open()){
			f_in_.close();
		}
		if(f_out_.is_open()){
			f_out_.close();
		}
	}
	void Drop(H264BinInfo*nalus,int *droptable,int drop_table_len){
		if(!f_in_.is_open()){
			std::cout<<"in error"<<std::endl;
			return;
		}
		if(!f_out_.is_open()){
			std::cout<<"out error"<<std::endl;
			return;
		}
		const std::map<int,NaluLogInfo>& infos=nalus->GetInfo();
		int dropped=0;
		for(auto it=infos.begin();it!=infos.end();it++){
			int no=it->first;
			int offset=it->second.offset;
			buf_.Reset(offset);
			f_in_.read(buf_.data(),offset);
			int len=f_in_.gcount();
			if(should_drop(no,droptable,drop_table_len)){
				dropped++;	
			}else{
				f_out_.write(buf_.data(),len);
			}
			if(dropped>=drop_table_len){
				break;
			}
		}
		CopyRemain();
	}
	void CopyRemain(){
		int buf_len=buf_.Len();
		for(;;){
			f_in_.read(buf_.data(),buf_len);
			int len=f_in_.gcount();
			//std::cout<<"read len "<<len<<std::endl;
			if(len>0){
				f_out_.write(buf_.data(),len);
			}
			if(len<=0){
				break;
			}
		}

	}
private:
	std::fstream f_in_;
	std::fstream f_out_;
	ReadBuffer buf_;
};
int main()
{
    std::string line;
    ifstream myfile("../1280x720_info.txt");
    H264BinInfo h264info;
    if(myfile.is_open()){
        while(myfile.good()){
            getline(myfile,line);
            std::vector<std::string> str_arry=split(line," ");
            if(str_arry.size()<=0){
                break;
            }
            std::string nalu_type=str_arry[4];
            int no=std::stoi(str_arry[0]);
            int start=std::stoi(str_arry[1]);
            int offset=std::stoi(str_arry[2]);
            int type=GetType(nalu_type);
            std::cout<<no<<" "<<type<<" "<<offset<<std::endl;
            NaluLogInfo info(no,start,offset,type);
            h264info.OnNewNaluInfo(info);
        }
    }
    myfile.close();
    std::string in("../1280x720.h264");
    std::string out("../out.h264");
    FrameDroper drop(in,out);
    drop.Drop(&h264info,drop_table,sizeof(drop_table));
    return 0;
}
