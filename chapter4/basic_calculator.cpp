#include<iostream>
#include<stack>
#include<vector>

using namespace std;

class Solution {
public:
    int calculate(string s) {
        //存放截止到括号之前计算得到的result
        stack<int> nums;
        //存放括号之前符号，用于后来走到括号边界，对括号内部的计算结果赋予符号
        stack<int> ops;
        int result = 0;
        int sign = 1;
        //对应输入的计算式中出现的数字
        int number = 0;
        //对于输入的每一个字符
        for (auto ch:s) {
            //如果该字符是数字字符
            if (isdigit(ch)) {
                //更新number
                number = number * 10 + ch - '0';
            }
            else {
                //不是数字，更新result
                result += sign * number;
                //数字number归零
                number = 0;
                //记录符号变化
                if (ch == '+')
                    sign = 1;
                if (ch == '-')
                    sign = -1;
                //遇到左括号
                if (ch == '(') {
                    //将括号之前的计算结果加入堆栈
                    nums.push(result);
                    //result归零，开始记录括号内部计算式的计算结果
                    result = 0;
                    //记录括号之前的符号
                    ops.push(sign);
                    //重置记录符号的sign
                    sign = 1;
                }
                //遇到了右括号 且堆栈不为空
                if (ch == ')' && nums.size()) {
                    //result为括号内部计算式的计算结果乘以括号左边的符号对应的正负，再加上括号之前的计算式的计算结果
                    result = ops.top() * result + nums.top();
                    ops.pop();
                    nums.pop();
                }
            }
        }

        result += sign * number;
        return result;
    }
};

int main() {
    Solution *s = new Solution();
    vector<string> strs{"1 + 1", " 2-1 + 2 ", "(1+(4+5+2)-3)+(6+8)"};
    for (auto str:strs) {
        cout << s->calculate(str) << endl;
    }
    return 0;
}