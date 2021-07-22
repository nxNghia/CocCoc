#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stack>

using namespace std;

const string FILE_NOT_EXISTED = "FILE_NOT_EXISTED";
const string STRANGE_CHARACTER = "SYNTAX_ERROR";

const string language_file = "currentLanguage.txt"; //file lưu tên file ngôn ngữ sử dụng
bool file_exist = false;

class Language
{
    private:
        map<string, string> numbers;
        map<string, string> suffixs;
        map<string, string> exception;
        map<string, string> alert;
        int max_length = 0; //biến này tùy vào ngôn ngữ sẽ có những giá trị khác nhau
        int max_expression = 0; //vị trí hậu tố lớn nhất (VD: trillion, tỷ, ...)
        bool isMinus = false;

        string getToken(string i_value, int pos)
        {
            string result;  //lưu hậu tố đằng sau giá trị i_value (ty, hundred, thousand, ...)

            if(pos % max_length == 0)   //số nằm tại các vị trí hàng đơn vị, hàng nghìn, hàng triệu, ...
            {
                if(pos > max_expression)   //trường hợp số hiện tại nằm ở vị trí không thể biểu diễn bằng các hậu tố đã cho
                {
                    result = suffixs[to_string(pos % max_expression)];
                }else{
                    result = suffixs[to_string(pos)];
                }
            }else{
                result = suffixs[to_string(pos % max_length)];   //số nằm tại các vị trí hàng chục, hàng trăm
            }
            
            string s_value = numbers[to_string(i_value.at(0) - '0')];

            if(pos % max_length == 0 && i_value == "0") //trong trường hợp số 0 ở hàng đơn vị => chỉ đọc hậu tố không đọc zero
                s_value = "";

            s_value += result;  //ghép hậu tố vào đằng sau giá trị chữ (four + hundred)
            result = "";    //xóa hậu tố
            s_value = exception.count(s_value) ? exception[s_value] : s_value;  //kiểm tra xem s_value có nằm trong danh sách những chữ phải biến đổi không

            //trong trường hợp số 1 ở vị trí hàng chục => truyền thêm số ở hàng đơn vị
            if(i_value.length() == 2)
            {
                if((pos - 1) % max_length == 0)
                {
                    result = suffixs[to_string(pos - 1)];
                }else{
                    result = suffixs[to_string((pos - 1) % max_length)];
                }
                s_value += " " + numbers[to_string(i_value.at(1) - '0')];
                s_value = exception.count(s_value) ? exception[s_value] : s_value;
            }

            return exception.count(s_value + result) ? exception[s_value + result] : s_value + result;
        }

        stack<string> convert(string number)
        {
            stack<string> result;
            int zero_count = 0;

            for (int i = 0; i < number.length(); i++)
            {
                if(number[i] == '1' && (number.length() - i - 1) % max_length == 1) //trường hợp số 1 ở hàng chục => truyền thêm số hàng đơn vị vào getToken
                {
                    result.push(getToken(number.substr(i, 2), number.length() - i - 1));
                    ++i;
                }
                else    //nếu không thì truyền vào số ở vị trí hiện tại
                    result.push(getToken(number.substr(i, 1), number.length() - i - 1));

                //đếm số 0 liên tiếp
                if(number[i] == '0')
                    ++zero_count;
                else
                    zero_count = 0;

                //nếu số lượng 0 lớn hơn max_length và vị trí hiện tại nằm tại hàng đơn vị => không đọc giá trị của max_length số đó
                //tránh đọc zero hundred zeroty zero
                if((number.length() - i - 1) % max_length == 0)
                {
                    if(zero_count == max_length)
                    {
                        //chỉ xóa max_length item trong stack
                        while(zero_count != 0)
                        {
                            result.pop();
                            --zero_count;
                        }
                    }
                    zero_count = 0;
                }
                    
            }

            return result;
        }

        //xóa các dấu cách thừa
        string beautify(string number)
        {
            int space = 1;

            for(string::iterator it = number.begin(); it != number.end();)
            {
                if(*it == ' ')
                    ++space;
                else
                    space = 0;
                
                if(space == 2)
                    number.erase(it);
                else
                    ++it;
            }

            return number;
        }

        string normalize(string number)
        {
            bool begin = false;

            if(number[0] == '-')
            {
                isMinus = true;
                number.erase(0, 1);
            }

            for(string::iterator it = number.begin(); it != number.end();)
            {
                if(*it < '0' || *it > '9')
                    return "";
                if(*it == '0' && begin == false)
                {
                    number.erase(number.begin());
                }else{
                    if(*it != '0' || begin)
                    {
                        ++it;
                        begin = true;
                    }
                }
            }

            return number;
        }

    public:
        Language(string filename)
        {
            ifstream f(filename);
            string str;
            string split = " ";

            file_exist = f.good();

            //đọc giá trị dạng chữ của các số 0, 1, 2, 3, ..., 9
            getline(f, str);
            //tách thành các xâu riêng lẻ rồi lưu vào numbers
            size_t pos = 0;
            string token;
            while((pos = str.find(split)) != string::npos)
            {
                token = str.substr(0, pos);
                numbers[to_string(numbers.size())] = token;
                str.erase(0, pos + 1);
            }
            numbers[to_string(numbers.size())] = str;

            //đọc các hậu tố (ty, hundred, thousand, mươi, trăm, nghìn, ...) rồi lưu vào suffixs
            int pre_token = 0;  //lưu hậu tố đã đọc
            split = ':';
            while(getline(f, str))
            {
                if(str == "")
                    break;

                pos = str.find(split);
                token = str.substr(0, pos);
                str.erase(0, pos + split.length());
                suffixs[str] = token;

                if(stoi(str) - pre_token > max_length)
                    max_length = stoi(str) - pre_token;

                pre_token = stoi(str);

                if(stoi(str) > max_expression)
                    max_expression = stoi(str);
            }

            //đọc các thông báo
            split = ":";
            while(getline(f, str))
            {
                if(str == "")
                    break;

                pos = str.find(split);
                token = str.substr(0, pos);
                str.erase(0, pos + split.length());
                alert[token] = str;
            }

            //đọc các ngoại lệ
            split = "->";
            while(getline(f, str))
            {
                size_t pos = str.find(split);
                string pre_word = str.substr(0, pos);
                str.erase(0, pos + split.length());
                exception[pre_word] = str;
            }
        }

        string getAlert(string alertType)
        {
            return alert[alertType];
        }

        string read(string number)
        {
            string result;
            number = normalize(number);
                if(number == "")  return getAlert(STRANGE_CHARACTER);

            stack<string> converted = convert(number);
            string minus = isMinus ? exception["-"] + " " : "";
            
            while(!converted.empty())
            {
                if(converted.top() != "")
                {
                    result = converted.top() + " " + result;
                }
                converted.pop();
            }

            return beautify(minus + result);
        }
};

int main()
{
    string number;
    cin >> number;
    ifstream f1(language_file);
    string language;
    getline(f1, language);
    Language used_language = Language("Languages/" + language);
    
    if(file_exist)
        cout << used_language.read(number) << endl;
    else
        cout << used_language.getAlert(FILE_NOT_EXISTED) << endl;
}