#include <iostream>  
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string.h>

#define blocksize  10000    //分段大小
#define coversize  200      //覆盖长度，需大于等于短串长度


using namespace std;  

//读取待匹配短串
string GetShortString(string filename)
{
	char *file = (char *)filename.data();
	ifstream in(file); 
	string s;	
	getline(in,s);
	//cout<<"s="<<s<<endl;
	in.close();
	return s;
}
//读取目标长串，之前已经读了pre_turns次长串
string GetLongString(string longfilename,int pre_turns){
	char *file = (char *)longfilename.data();
	ifstream in(file);
	string temp; 
	getline(in,temp);    //跳过第一行
	if(pre_turns!=0) {
		in.seekg(pre_turns*(blocksize-coversize),ios::cur);    //寻址到目标长串起始位置
	}		
	ostringstream buf;
	char ch;
	while(buf){
		in.get(ch);
		if(ch<65||ch>90)    //不是ACGT
			break;
		buf.put(ch);
		if(buf.str().size() == blocksize)
		break;
		
	}
	in.close();
	return buf.str();
}

//求OCC[case1][pos]
int GetOCC(int case1, int pos,int **OCC,string lastcolumn){ 
	int k = pos/32;
	if(pos%32 == 0) 
		return OCC[case1][k];

	char ch;
	if(case1 == 0) ch='A';
	else if (case1 ==1) ch='C';
	else if (case1 ==2) ch='G';
	else if (case1 ==3) ch='T';

	int result = OCC[case1][k];
	for(int j=32*k; j<pos; j++){
		if(lastcolumn[j] == ch)
			result++;
	}
	return result;
}

//求suffix[pos]
int GetSuffix(int pos,int *suffix,int *C,int **OCC,string lastcolumn){ 
	int k = pos/32;
	if(pos%32 == 0) 
		return suffix[k];
	int case1;

	char ch=lastcolumn[pos];
	if(ch=='A') case1=0;
	else if(ch=='C') case1=1;
	else if(ch=='G') case1=2;
	else if(ch=='T') case1=3;

	int lastpos = C[case1]+GetOCC(case1,pos,OCC,lastcolumn);
	return GetSuffix(lastpos,suffix,C,OCC,lastcolumn)+1 ;
}


void BWT_MATCH(string longstring,string shortstring,int *C,int **OCC,int *suffix_array,string lastcolumn,int max1,int row){
	int len=longstring.length();
	int offset;
	string temp1;
	
	int cur = shortstring.length()-1;
	int sp,ep;
	char Last_OF_ShortString = shortstring[cur];
	int case1;
	if (Last_OF_ShortString == 'A') case1 =0;
	else if (Last_OF_ShortString == 'C') case1 =1;
	else if (Last_OF_ShortString == 'G') case1 =2;
	else if (Last_OF_ShortString == 'T') case1 =3;
	sp = C[case1];
	ep = C[case1 +1]-1;

	while(sp<=ep){	
		if (shortstring[cur-1] == 'A') case1 =0;
		else if (shortstring[cur-1] == 'C') case1 =1;
		else if (shortstring[cur-1] == 'G') case1 =2;
		else if (shortstring[cur-1] == 'T') case1 =3;
		cur --;
		Last_OF_ShortString = shortstring[cur];
		int newsp = C[case1] + GetOCC(case1,sp,OCC,lastcolumn);
		int newep = newsp + GetOCC(case1,ep+1,OCC,lastcolumn) - GetOCC(case1,sp,OCC,lastcolumn) -1;
		sp = newsp;
		ep = newep;
	
		if(cur<=0) {
			if(sp<=ep){
				cout<<"Succeed! There is/are "<<ep-sp+1<<" match places with offset(s):"<<endl;
				for(int hh = sp; hh <= ep; hh++)
					cout<<"sp="<<sp<<","<<GetSuffix(hh,suffix_array,C,OCC,lastcolumn)+row*(blocksize-coversize)<<endl;
			}
			else cout<<"No matched result."<<endl;
			break;
		}
	} //end while
}

int main()  
{  
	string shortfilename = "search_SRR00001test.txt"; // 待匹配短串文件
	string longfilename = "SRR00001_rows_ATCG.txt";     // 长串文件
	string shortstring = GetShortString(shortfilename);  //读取短串
	string  longstring;
	int i,k;

	ifstream in(longfilename); 
	string s;
	getline(in,s);
	in.close();
	int longStringLength = atoi(s.c_str());

	ifstream datafile("suffixArray1new.txt");
	string datastream;
	
	for(i=0;i*(blocksize-coversize)<longStringLength;i++){
		getline(datafile,datastream); //datastream是一整行数据，依次包括C[0...4], s[0],OCC[0][0...3] ...
		stringstream value(datastream);
		string temp1;

		longstring = GetLongString(longfilename,i);  //长串
		int len=longstring.length();
		int *suffix_array = (int *)malloc((len/32+1)*sizeof(int));
		int *C=(int *)malloc(5*sizeof(int));	
		int **OCC=(int **)malloc(4*sizeof(int *));
		for(k=0;k<4;k++){
			OCC[k]=(int *)malloc((len/32+1)*sizeof(int));
			memset(OCC[k],0,(len/32+1)*sizeof(int));
		}

		for(int j=0; j<5; j++){  //首先读取C数组
			value >> temp1;
			C[j] = atoi(temp1.c_str());
		}	

		//读取suffix数组和OCC数组	
		int max1;
		if (len%32 == 0) max1 = len/32;
		else max1 = len/32 + 1;
		for(int j=0; j<max1 ; j++){
			value >> temp1;  suffix_array[j] = atoi(temp1.c_str());
			value >> temp1;  OCC[0][j] = atoi(temp1.c_str());
			value >> temp1;  OCC[1][j] = atoi(temp1.c_str());
			value >> temp1;  OCC[2][j] = atoi(temp1.c_str());
			value >> temp1;  OCC[3][j] = atoi(temp1.c_str());
		}		
		string lastcolumn;
		value >> lastcolumn;

		//匹配短串
		BWT_MATCH(longstring,shortstring,C,OCC,suffix_array,lastcolumn,max1,i);
		for(k=0;k<4;k++) free (OCC[k]);
		free(OCC);
		free(C);
		free(suffix_array);
	}  // end i 
	datafile.close();
} // end main

