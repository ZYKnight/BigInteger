#include "BigInt.h"
#include "iostream"
#include "algorithm"
#include "cstdio"
#include "cassert"
using namespace std;
void BigInt::clearZero() {
	//清除最高位的零
	while(s.size()>1 && s.back()==0)
		s.pop_back();
}


bool legal(const string &str) {
	int len=str.length();
	int i=0; 
	if(str[0]=='-')
		i++;
	for(;i<len;i++) {
		if(str[i]<'0'||str[i]>'9')
		{
			cout<<"string is illegal"<<endl;
			return false;
		}
	}
	return true;
}


BigInt& BigInt::operator = (const string &str) {
	if(legal(str)) {
		s.clear();
		int x,len=(str.length()-1)/WIDTH+1;
		if(str=="-0") //避免负零出现
			negative=0;
		negative=(str[0]=='-');
		int ss=negative;
		for(int i=0;i<len;i++) {
			//begin with lowest 8 nums
			int end=str.length() - i*WIDTH;
			int start=max(ss,end-WIDTH);
			sscanf(str.substr(start,end-start).c_str(),"%d",&x);
			s.push_back(x);
		}		
	}
	return *this;
}



BigInt& BigInt::operator = (const BigInt &a) {
	if(a == *this)
		return *this;
	int len=a.s.size();
	s.resize(len);
	//adjust size of vector to advoid memory leaking
	for(int i=0;i<len;i++) {
		s[i]=a.s[i];
	}
	negative=a.negative;
	return *this;
}


BigInt& BigInt::operator = (long long num) {
	s.clear();
	negative=(num<0);//保证只有 正数0
	do {
		s.push_back(num%BASE);
		num/=BASE;
	}while(num>0);

	return *this;
}


ostream& operator << (ostream &out, const BigInt &x) {
	int len=x.s.size();
	if(x.negative)
		out<<'-';
	out<<x.s.back(); //最高位不填充0
	for(int i=len-2;i>=0;i--) {
		char buf[16];
		//每一段长度为8，长度不足8则在首部填充0
		sprintf(buf,"%08d",x.s[i]);
		out<<buf; 
	}
	return out;
}


istream& operator >> (istream &in, BigInt &x) {
	string str;
	in>>str;
	x=str;
	return in;
}


BigInt BigInt::operator - () const {
	//reload unary operator -
	BigInt tmp(*this);
	tmp.negative= !negative;
	return tmp; 
}


BigInt BigInt::operator + (const BigInt &b) const {
	BigInt tmp;
	tmp.s.clear();
	tmp.negative = 0;
	if(negative && b.negative) 
		tmp.negative = 1;
	else if(negative && b.negative==0) {
		tmp.negative = -*this > b;
	}
	else if(negative==0 && b.negative) {
		tmp.negative = -b > *this;
	}
	else
		tmp.negative = 0;

	if(negative == b.negative) {
		int len_a = s.size();
		int len_b = b.s.size();
		for(int i=0,carry=0;;i++) {
			if(carry == 0 && i > len_a && i > len_b)
				break;
			int x=carry;
			if(i<len_a)    x+=s[i];
			if(i<len_b)	   x+=b.s[i];
			tmp.s.push_back(x % BASE);
			carry=x / BASE;
		}
	}
	else {
		BigInt mmin,mmax;
		if(-*this == b)
			return BigInt(0);
		//让绝对值大的数是正数，绝对值小的是负数
		if(negative) {
			if(-*this > b) { mmin = b; mmax = *this;}
			else { mmax = b; mmin = *this;}
		}
		else {
			if(*this > -b) { mmin = b; mmax = *this;}
			else { mmax = b; mmin = *this;}
		}
		int len_a = mmax.s.size();
		int len_b = mmin.s.size();
		for(int i=0,carry=0; ;i++) {
			if(carry == 0 && i > len_a && i > len_b)
				break;
			int x=carry;
			if(i<len_a)
				x+=mmax.s[i];
			if(i<len_b)
				x-=mmin.s[i];
			
			if(x<0) {
				//绝对值大的是 正数，不会出现无限借位的情况
				carry=-1;
				x=BASE+x;
			}
			else 
				carry=0;
			tmp.s.push_back(x % BASE);
		}
		
	}
	//清除最高位的0
	tmp.clearZero();
	return tmp;
}


BigInt BigInt::operator += (const BigInt &b) {
	return *this = *this + b;
}


BigInt BigInt::operator - (const BigInt &b) const {
	
	return *this+(-b);
}


BigInt BigInt::operator -= (const BigInt &b) {
	return *this = *this - b;
}


BigInt BigInt::operator * (const __int64 &b) const {
	//a wheel for multiplication in BigInt
	
	assert(b<BASE);
	BigInt tmp;
	tmp.s.clear();
	if(!b)
		return tmp = 0;
	int len = s.size(), carry = 0;
	for(int i = 0; i < len; i++) {
		int x = carry;
		//b<BASE,保证 b*s[i] ∈ long long
		__int64 a = s[i] * b;
		x += a % BASE;
		carry = a / BASE;
		tmp.s.push_back(x);
	}
	if(carry)
		tmp.s.push_back(carry);
	tmp.clearZero();
	tmp.negative=0;
	return tmp;
}


BigInt BigInt::operator * (const BigInt &b) const {

	if(b==0)
		return 0;
	BigInt res;
	int lena = s.size();
	res = b * s[0];
	for(int i=1; i<lena; i++) {
		BigInt a = b * s[i]; //模拟乘法的手算，10^9进制
		a.s.resize(a.s.size() + i);
		int len = a.s.size();
		//低位补零，模拟左移
		while(len-- > i) {
			a.s[len] = a.s[len-i];
		}
		for(int j=0;j<i;j++)
			a.s[j]=0;
		//结果累加
		res += a;
	}
	res.clearZero();

	res.negative = (negative != b.negative);
	return res;
}


BigInt BigInt::operator *= (const BigInt &b) {
	return *this = *this * b;
}


BigInt BigInt::operator / (const BigInt &b) const {
	if(b==0) {
		cout<<"div zero!"<<endl;
		return 0;
	}
	if(*this < b)
		return 0;
	BigInt res,remain;
	//默认构造函数 构造为正0
	res.s.clear();
	remain.s.clear();
	remain.negative=0;
	for(int i=s.size()-1; i>=0; i--) {
		//每趟余数不会比b多8位以上，r 一定是int范围内的整数
		int r=0;
		remain.s.push_back(s[i]);
		int len=remain.s.size();
		while(len-->1) {
			remain.s[len] = remain.s[len-1];
		}
		remain.s[0] = s[i];
		while(remain >= b) {
			int btmp = 1<<31-1;
			//b是int范围内的整数，避免b太小导致调用减法次数过多
			if(b.s.size() == 1)
				btmp = b.s[0];
			if(remain.s[0] > btmp) {
				//tp*b是被减数
				int tp = remain.s[0]/btmp;
				if(tp == 0) 
					tp = 1;
				while(remain >= btmp) {
					if(remain >= tp*btmp) {
						remain -= tp*btmp;
						r+=tp;
					}
					else
						tp=(tp/10)?(tp/10):1;
				}
			}
			else {
				r++;
				remain -= b;
			}
		}
		res.s.push_back(r);
		if(remain==0)
			remain.s.clear();
	}
	for(int i=0, j=res.s.size()-1 ; i<j; i++,j--) {
		int tmp=res.s[i];
		res.s[i]=res.s[j];
		res.s[j]=tmp;
	}
	res.clearZero();
	res.negative = (negative != b.negative) && res!=0;
	return res;
}


BigInt BigInt::operator /= (const BigInt &b) {
	return *this = *this / b;
}


bool BigInt::operator !() const {
	return *this==0;
}


BigInt BigInt::operator % (const BigInt &b) const{
	if(b==0) {
		cout<<"div zero!"<<endl;
		return 0;
	}
	if(*this < b)
		return *this;
	BigInt remain;
	//默认构造函数 构造为正0
	remain.s.clear();
	remain.negative=0;
	for(int i=s.size()-1; i>=0; i--) {
		remain.s.push_back(s[i]);
		int len=remain.s.size();
		while(len-- > 1) {
			remain.s[len] = remain.s[len-1];
		}
		remain.s[0] = s[i];
		remain.clearZero();
		while(remain >= b) {
			int btmp = 1<<31-1;
			//b是int范围内的整数，避免b太小导致调用减法次数过多
			if(b.s.size() == 1)
				btmp = b.s[0];
			if(remain.s[0] > btmp) {
				//tp*b是被减数
				int tp = remain.s[0]/btmp;
				if(tp == 0) 
					tp = 1;
				while(remain >= btmp) {
					if(remain >= tp*btmp) {
						remain -= tp*btmp;
					}
					else
						tp=(tp/10)?(tp/10):1;
				}
			}
			else {
				remain -= b;
			}
		}
	}
	return remain;
}


BigInt BigInt::operator %= (const BigInt &b){
	return *this = *this % b;
}


BigInt& BigInt::operator ++() {
	int carry=1,len=s.size();
	if(negative) 
		carry=-1;
	for(int i=0;i<len;i++) {
		if(carry == 0)
			break;
		s[i]+=carry;
		carry=s[i]/BASE;
		s[i]=s[i]%BASE;
	}
	return *this;
}


const BigInt BigInt::operator ++(int) {
	BigInt tmp(*this);
	++*this;
	return tmp;
}


bool BigInt::operator >(const BigInt &b) const {
	//两数异号
	if(negative != b.negative)
		//b是负数，*this>b;b是正数，*this<b
		return b.negative;

	int len=b.s.size();
	//两个都是正数
	if(negative==0 && b.negative==0) {
		if(s.size() != len)
			return s.size()>len;
		while(len--)
		{
			if(s[len]!=b.s[len])
				return s[len]>b.s[len];
		}
	}
	//两个都是负数
	else if(negative && b.negative) {
		if(s.size() != len)
			return s.size()<len;
		while(len--)
		{
			if(s[len]!=b.s[len])
				return s[len]<b.s[len];
		}
	}
	//两个数相等
	return false;
}


bool BigInt::operator <(const BigInt &b) const {
	//两数异号
	if(negative != b.negative)
		//*this是负数，肯定小于b；是正数，肯定大于b
		return negative;

	int len=b.s.size();
	//两个都是正数
	if(negative == 0 && b.negative == 0) {
		if(s.size() != len)
			return s.size() < len;
		while(len--)
		{
			if(s[len] != b.s[len])
				return s[len] < b.s[len];
		}
	}
	//两个都是负数
	else if(negative && b.negative) {
		if(s.size() != len)
			return s.size() > len;
		while(len--)
		{
			if(s[len] != b.s[len])
				return s[len] > b.s[len];
		}
	}
	//两数相等的情况
	return false;
}


bool BigInt::operator == (const BigInt &b) const {
	//不考虑效率，可以写 return !(*this<b) && !(*this>b);
	int len=b.s.size();
	if(len!=s.size() || negative!=b.negative)
		return false;
	while(len--) {
		if(s[len] != b.s[len])
			return false;
	}
	return true;
}


bool BigInt::operator !=(const BigInt &b) const {
	return !(*this == b);
}


bool BigInt::operator >= (const BigInt &b) const {
	//*this < b与*this >= b互补 
	return !(*this < b);
}


bool BigInt::operator <=(const BigInt &b) const {
	//*this > b与*this <= b互补 
	return !(*this > b);
}


BigInt::~BigInt() {
	s.clear();//不写这个心慌。。。
}


//printf numbers in vector<int> s,for test

void BigInt::printVector() {
	int len=s.size();
	cout<<"numbers in vector: ";
	for(int i=0;i<len-1;i++)
		cout<<s[i]<<',';
	cout<<s[len-1]<<endl;
}


string BigInt::tostring() const{
	ostringstream out;
	out.clear();
	int len=s.size();
	if(negative)
		out<<'-';
	out<<s.back(); //最高位不填充0
	for(int i=len-2;i>=0;i--) {
		char buf[16];
		//每一段长度为8，长度不足8则在首部填充0
		sprintf(buf,"%08d",s[i]);
		out<<buf; 
	}
	return out.str();
}
