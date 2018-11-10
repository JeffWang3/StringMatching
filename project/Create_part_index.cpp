#include <iostream>  
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#define blocksize  10000
#define coversize  200

using namespace std;  

//快速排序获得suffix array数组 
void quickSort(int s[], int l, int r, string sentence)  
{  	
	int k;
	int len = sentence.length()/2;
    if (l< r) {    
        int i = l, j = r; 
        int x = s[l];
        string standard = sentence.substr(s[l],len);  
        while (i < j) {  
            while(i < j && sentence.substr(s[j],len).compare(standard)>=0)
				j--; 
            if(i < j)
            	s[i++] = s[j];      
            while(i < j && sentence.substr(s[i],len).compare(standard)<0) 
                i++;   
            if(i < j) 
            	s[j--] = s[i];  
    	}
        s[i] = x;  
        quickSort(s, l, i - 1,sentence);  
        quickSort(s, i + 1, r,sentence);  
    }  
} 
//部分存储s,C,OCC。每隔32位存一次。比如s[0],s[32],s[64]... 
void SAVE(int s[],int *C, int **OCC, string lastcolumn, int len){ 
	ofstream file;
 	file.open("suffixArray2new.txt",ios::app); 
 	file<<C[0]<<" "<<C[1]<<" "<<C[2]<<" "<<C[3]<<" "<<C[4]<<" ";   //先存C数组的五个元素
	for(int k=0;k<len;k++) {
		if(k%32==0){
    		file<<s[k]<<" ";
    		file<<OCC[0][k]<<" "<<OCC[1][k]<<" "<<OCC[2][k]<<" "<<OCC[3][k]<<" ";
		}
    } 
    file<<lastcolumn<<" "<<endl;
    file.close();
}
//读取目标长串，之前已经读了pre_turns次长串
string GetLongString(string longfilename,int pre_turns){
	char *file = (char *)longfilename.data();
	ifstream in(file);
	string temp; 
	getline(in,temp);   //跳过第一行
	if(pre_turns!=0) {
		in.seekg(pre_turns*(blocksize-coversize),ios::cur);     //寻址到目标长串起始位置
	}	
	ostringstream buf;
	char ch;
	//读取目标长串
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

//计算C数组和OCC数组
void Count_C_OCC(string longstring,int C[],int **OCC, int *suffix_array){ 
	int len=longstring.length();
	int offset;
	//C[0,1,2,3]分别对应ACGT的起始位置。OCC[4]是T的结束位置+1
	C[0]=0; C[1]=0; C[2]=0; C[3]=0; C[4]=0; 
	for(int j=0; j<len; j++){	
		offset = suffix_array[j];		
		char begin = longstring[offset];
		char end = longstring[(offset+len-1)%len];
		switch(begin){
			case 'A':C[1]++;C[2]++;C[3]++;C[4]++; 
				 break;
			case 'C':C[2]++;C[3]++;C[4]++; 						 
				 break;
			case 'G':C[3]++;C[4]++; 
				 break;
			case 'T':C[4]++; 
				 break;
		}
		OCC[0][j+1]=OCC[0][j];
		OCC[1][j+1]=OCC[1][j];
		OCC[2][j+1]=OCC[2][j];
		OCC[3][j+1]=OCC[3][j];
		switch(end){
			case 'A':
				OCC[0][j+1]+=1;
				break;
			case 'C':
				OCC[1][j+1]+=1;					 
				break;
			case 'G':
				OCC[2][j+1]+=1;
				break;
			case 'T':
				OCC[3][j+1]+=1;
				break;
			default:
				break;
		}
	}  // end for j		
}


int main()  
{  
	string shortfilename = "search_SRR00001test.txt"; //自己的测试文件。注意只能有一个短串
	string longfilename = "SRR163132_rows_ATCG.txt";  //长串文件
	string  longstring;
	int i;

	//获取长串长度
	int longStringLength;
	char *file = (char *)longfilename.data();
	ifstream in(file); 
	string s;
	getline(in,s);
	in.close();
	longStringLength = atoi(s.c_str());
	cout<<"here:"<<longStringLength<<endl;
	//主循环，每次对长度为10000的长串子段创建索引，并存储到本地 
	for(i=0;i*(blocksize-coversize)<longStringLength;i++){ 
        //生成suffix array
		longstring = GetLongString(longfilename,i);  //得到长串
		string doublelongstring = longstring;   //不用生成矩阵的快排的策略
		doublelongstring+=longstring; 
    	int len=longstring.length(); 
		int* suffix_array= (int *)malloc(sizeof(int)*len);
		int k;
		for(k=0;k<len;k++)
			suffix_array[k]=k;   
    	quickSort(suffix_array,0,len-1,doublelongstring); 
		
    	//生成c和occ数组
		int *C=(int *)malloc(5*sizeof(int));	
		int **OCC=(int **)malloc(4*sizeof(int *));
		for(k=0;k<4;k++){
			OCC[k]=(int *)malloc((len+1)*sizeof(int));
			memset(OCC[k],0,(len+1)*sizeof(int));
		}		
		Count_C_OCC(longstring, C, OCC, suffix_array);
		string lastcolumn;
		for(int j=0; j <len; j++){
			lastcolumn+=longstring[(suffix_array[j]+len-1)%len];
		}
		//保存到本地 
		SAVE(suffix_array, C, OCC, lastcolumn, len); 
		free (suffix_array);
		for(k=0;k<4;k++) free (OCC[k]);
		free(OCC);
		free(C);
	}
    return 0;  
}   
