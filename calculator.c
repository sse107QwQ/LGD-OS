// calculator.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "declarations.h"

// 计算器状态结构
typedef struct {
    double memory;  // 内存值
    int use_radians; // 是否使用弧度
} CalculatorState;

// 计算器状态
static CalculatorState calc_state = {0, 0};

// 帮助函数：判断是否是运算符
static int is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '[';
}

// 帮助函数：获取运算符优先级
static int get_precedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '^':
            return 3;
        case '[':  // 开方运算
            return 4;
        default:
            return 0;
    }
}

// 帮助函数：执行运算
static double apply_operation(double a, double b, char op) {
    switch (op) {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            if (b == 0) {
                fprintf(stderr, "错误: 除以零\n");
                return 0;
            }
            return a / b;
        case '^':
            return pow(a, b);
        case '[':  // 开方：b 开 a 次方
            if (a <= 0) {
                fprintf(stderr, "错误: 根号运算的根指数必须大于0\n");
                return 0;
            }
            if (b < 0 && fmod(a, 2) == 0) {
                fprintf(stderr, "错误: 负数不能开偶次方\n");
                return 0;
            }
            return pow(b, 1.0 / a);
        default:
            return 0;
    }
}

// 帮助函数：计算表达式
static double evaluate_expression(const char *expr) {
    double numbers[100];
    char operators[100];
    int num_top = -1, op_top = -1;
    int i = 0;
    int negative = 0;

    while (expr[i] != '\0') {
        // 跳过空格
        if (isspace(expr[i])) {
            i++;
            continue;
        }

        // 处理负数
        if (expr[i] == '-' && (i == 0 || expr[i-1] == '(' || is_operator(expr[i-1]))) {
            negative = 1;
            i++;
            continue;
        }

        // 处理数字
        if (isdigit(expr[i]) || expr[i] == '.') {
            double num = 0;
            int decimal = 0;
            double fraction = 1.0;

            // 整数部分
            while (isdigit(expr[i])) {
                num = num * 10 + (expr[i] - '0');
                i++;
            }

            // 小数部分
            if (expr[i] == '.') {
                decimal = 1;
                i++;
                while (isdigit(expr[i])) {
                    fraction *= 0.1;
                    num += (expr[i] - '0') * fraction;
                    i++;
                }
            }

            // 应用负号
            if (negative) {
                num = -num;
                negative = 0;
            }

            // 将数字压入栈
            if (num_top < 99) {
                numbers[++num_top] = num;
            }
            continue;
        }

        // 处理开方运算符
        if (expr[i] == '[') {
            // 检查是否是二元开方运算
            if (num_top >= 0 && i > 0 && (isdigit(expr[i-1]) || expr[i-1] == ')')) {
                // 这是二元开方
                if (op_top < 99) {
                    operators[++op_top] = '[';
                }
                i++;
            } else {
                // 这是一元开方
                if (op_top < 99) {
                    operators[++op_top] = '[';
                }
                // 将2压入栈作为默认的根指数（平方根）
                if (num_top < 99) {
                    numbers[++num_top] = 2;
                }
                i++;
            }
            continue;
        }

        // 处理左括号
        if (expr[i] == '(') {
            if (op_top < 99) {
                operators[++op_top] = expr[i];
            }
            i++;
            continue;
        }

        // 处理右括号
        if (expr[i] == ')') {
            while (op_top >= 0 && operators[op_top] != '(') {
                if (num_top < 1) {
                    fprintf(stderr, "错误: 表达式无效\n");
                    return 0;
                }
                double b = numbers[num_top--];
                double a = numbers[num_top--];
                char op = operators[op_top--];
                double result = apply_operation(a, b, op);
                if (num_top < 99) {
                    numbers[++num_top] = result;
                }
            }
            if (op_top >= 0 && operators[op_top] == '(') {
                op_top--;  // 弹出左括号
            }
            i++;
            continue;
        }

        // 处理运算符
        if (is_operator(expr[i]) && expr[i] != '[') {
            while (op_top >= 0 && get_precedence(operators[op_top]) >= get_precedence(expr[i])) {
                if (num_top < 1) {
                    fprintf(stderr, "错误: 表达式无效\n");
                    return 0;
                }
                double b = numbers[num_top--];
                double a = numbers[num_top--];
                char op = operators[op_top--];
                double result = apply_operation(a, b, op);
                if (num_top < 99) {
                    numbers[++num_top] = result;
                }
            }
            if (op_top < 99) {
                operators[++op_top] = expr[i];
            }
            i++;
            continue;
        }

        // 未知字符
        fprintf(stderr, "错误: 无效字符 '%c'\n", expr[i]);
        return 0;
    }

    // 处理剩余的运算符
    while (op_top >= 0) {
        if (num_top < 1) {
            fprintf(stderr, "错误: 表达式无效\n");
            return 0;
        }
        double b = numbers[num_top--];
        double a = numbers[num_top--];
        char op = operators[op_top--];

        // 处理一元运算符
        if (op == '[' && num_top < 0) {
            // 一元开方
            double result = apply_operation(a, b, op);
            if (num_top < 99) {
                numbers[++num_top] = result;
            }
        } else {
            double result = apply_operation(a, b, op);
            if (num_top < 99) {
                numbers[++num_top] = result;
            }
        }
    }

    if (num_top != 0) {
        fprintf(stderr, "错误: 表达式无效\n");
        return 0;
    }

    return numbers[num_top];
}

// 简单方程解析函数 - 新版本
static int parse_equation_simple(const char *equation_str, double *a, double *b, double *c, FILE *out) {
    char eq_copy[256];
    int len = strlen(equation_str);
    int j = 0;

    // 复制并转换为小写
    for (int i = 0; i < len && j < 255; i++) {
        eq_copy[j++] = tolower(equation_str[i]);
    }
    eq_copy[j] = '\0';

    // 找到等号
    char *equal_sign = strchr(eq_copy, '=');
    if (!equal_sign) {
        fprintf(out, "错误: 方程必须包含等号\n");
        return 0;
    }

    *equal_sign = '\0';
    char *left_side = eq_copy;
    char *right_side = equal_sign + 1;

    // 初始化系数
    *a = 0.0;
    *b = 0.0;
    *c = 0.0;

    // 解析右侧，如果是0就忽略，如果不是0就移到左边
    double right_val = 0.0;
    if (right_side[0] != '\0') {
        char *endptr;
        right_val = strtod(right_side, &endptr);
        if (endptr == right_side) {
            // 解析失败
            right_val = 0.0;
        }
    }

    // 处理左边表达式
    char *ptr = left_side;
    double current_coeff = 1.0;
    int is_negative = 0;
    int has_digit = 0;
    double num = 0.0;
    int decimal_point = 0;
    double decimal_div = 1.0;

    // 解析整个表达式
    while (*ptr) {
        if (*ptr == '+' || *ptr == '-') {
            // 保存前一项
            is_negative = (*ptr == '-');
            ptr++;
            continue;
        }

        // 解析数字
        if (isdigit(*ptr) || *ptr == '.') {
            num = 0.0;
            decimal_point = 0;
            decimal_div = 1.0;
            has_digit = 1;

            while (isdigit(*ptr) || *ptr == '.') {
                if (*ptr == '.') {
                    decimal_point = 1;
                } else {
                    if (decimal_point) {
                        decimal_div *= 10.0;
                        num = num + (*ptr - '0') / decimal_div;
                    } else {
                        num = num * 10.0 + (*ptr - '0');
                    }
                }
                ptr++;
            }

            if (is_negative) {
                num = -num;
                is_negative = 0;
            }

            // 检查下一项是什么
            if (*ptr == 'x') {
                ptr++;  // 跳过'x'
                if (*ptr == '^' && *(ptr+1) == '2') {
                    // 这是x^2项
                    *a += num;
                    ptr += 2;  // 跳过'^2'
                } else {
                    // 这是x项
                    *b += num;
                }
            } else {
                // 常数项
                *c += num;
            }

            has_digit = 0;
            continue;
        }

        // 处理单个'x'
        if (*ptr == 'x') {
            double coeff = 1.0;
            if (is_negative) {
                coeff = -1.0;
                is_negative = 0;
            }

            ptr++;  // 跳过'x'

            if (*ptr == '^' && *(ptr+1) == '2') {
                // 这是x^2项
                *a += coeff;
                ptr += 2;  // 跳过'^2'
            } else {
                // 这是x项
                *b += coeff;
            }
            continue;
        }

        // 跳过未知字符
        ptr++;
    }

    // 从c中减去右侧的值
    *c -= right_val;

    return 1;
}

// 解一元一次方程
static int solve_linear_equation(double a, double b, FILE *out) {
    fprintf(out, "\n=== 实验功能：解一元一次方程 ===\n");
    fprintf(out, "方程: %.2fx + %.2f = 0\n", a, b);

    if (fabs(a) < 1e-10) {
        if (fabs(b) < 1e-10) {
            fprintf(out, "解: 无穷多解 (任意实数)\n");
        } else {
            fprintf(out, "解: 无解\n");
        }
    } else {
        double x = -b / a;
        fprintf(out, "解: x = %.10f\n", x);
    }

    fprintf(out, "====================================\n");
    return 1;
}

// 解一元二次方程
static int solve_quadratic_equation(double a, double b, double c, FILE *out) {
    fprintf(out, "\n=== 实验功能：解一元二次方程 ===\n");
    fprintf(out, "方程: %.2fx^2 + %.2fx + %.2f = 0\n", a, b, c);

    if (fabs(a) < 1e-10) {
        fprintf(out, "注意: a约等0，这实际上是一元一次方程\n");
        return solve_linear_equation(b, c, out);
    }

    // 计算判别式
    double delta = b * b - 4 * a * c;

    fprintf(out, "判别式: %.10f\n", delta);

    if (delta > 1e-10) {
        // 两个实数解
        double sqrt_delta = sqrt(delta);
        double x1 = (-b + sqrt_delta) / (2 * a);
        double x2 = (-b - sqrt_delta) / (2 * a);

        fprintf(out, "解: 两个不同的实数解\n");
        fprintf(out, "x1 = %.10f\n", x1);
        fprintf(out, "x2 = %.10f\n", x2);
    } else if (fabs(delta) < 1e-10) {
        // 一个实数解
        double x = -b / (2 * a);
        fprintf(out, "解: 一个实数解（重根）\n");
        fprintf(out, "x = %.10f\n", x);
    } else {
        // 无实数解
        delta = -delta;  // 取绝对值
        fprintf(out, "解: 无实数解（有两个共轭复数解）\n");
        double real_part = -b / (2 * a);
        double imag_part = sqrt(delta) / (2 * a);

        fprintf(out, "复数解:\n");
        if (fabs(real_part) < 1e-10) real_part = 0.0;
        if (fabs(imag_part) < 1e-10) imag_part = 0.0;

        fprintf(out, "x1 = %.6f + %.6fi\n", real_part, imag_part);
        fprintf(out, "x2 = %.6f - %.6fi\n", real_part, imag_part);
    }

    fprintf(out, "========================================\n");
    return 1;
}
// 高斯消元法求解线性方程组
static int gaussian_elimination(double **matrix, int n, double *solution, FILE *out) {
    int i, j, k;

    // 显示增广矩阵
    fprintf(out, "\n增广矩阵:\n");
    for (i = 0; i < n; i++) {
        fprintf(out, "[ ");
        for (j = 0; j < n; j++) {
            fprintf(out, "%8.3f ", matrix[i][j]);
        }
        fprintf(out, "| %8.3f ]\n", matrix[i][n]);
    }

    // 前向消元
    for (i = 0; i < n; i++) {
        // 寻找主元
        int max_row = i;
        for (j = i + 1; j < n; j++) {
            if (fabs(matrix[j][i]) > fabs(matrix[max_row][i])) {
                max_row = j;
            }
        }

        // 如果主元为0，尝试在后续列寻找
        if (fabs(matrix[max_row][i]) < 1e-10) {
            int found = 0;
            for (k = i + 1; k < n; k++) {
                if (fabs(matrix[max_row][k]) > 1e-10) {
                    // 交换列
                    for (j = 0; j < n; j++) {
                        double temp = matrix[j][i];
                        matrix[j][i] = matrix[j][k];
                        matrix[j][k] = temp;
                    }
                    fprintf(out, "提示: 交换了列 %d 和列 %d\n", i + 1, k + 1);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                // 检查是否无解
                if (fabs(matrix[max_row][n]) > 1e-10) {
                    fprintf(out, "错误: 方程组无解（矛盾方程）\n");
                    return 0;
                } else {
                    fprintf(out, "警告: 方程组有无穷多解\n");
                    // 继续处理，但跳过当前行
                    continue;
                }
            }

            // 重新寻找主元
            max_row = i;
            for (j = i + 1; j < n; j++) {
                if (fabs(matrix[j][i]) > fabs(matrix[max_row][i])) {
                    max_row = j;
                }
            }
        }

        // 交换行
        if (max_row != i) {
            double *temp = matrix[i];
            matrix[i] = matrix[max_row];
            matrix[max_row] = temp;
            fprintf(out, "交换行 %d 和行 %d\n", i + 1, max_row + 1);
        }

        // 主元归一化
        double pivot = matrix[i][i];
        fprintf(out, "主元: matrix[%d][%d] = %.6f\n", i + 1, i + 1, pivot);

        for (j = i; j <= n; j++) {
            matrix[i][j] /= pivot;
        }

        // 消去下方元素
        for (j = i + 1; j < n; j++) {
            double factor = matrix[j][i];
            if (fabs(factor) > 1e-10) {
                for (k = i; k <= n; k++) {
                    matrix[j][k] -= factor * matrix[i][k];
                }
                fprintf(out, "行%d -= %.3f * 行%d\n", j + 1, factor, i + 1);
            }
        }
    }

    // 回代求解
    for (i = n - 1; i >= 0; i--) {
        solution[i] = matrix[i][n];
        for (j = i + 1; j < n; j++) {
            solution[i] -= matrix[i][j] * solution[j];
        }

        // 检查对角线元素是否为0
        if (fabs(matrix[i][i]) < 1e-10) {
            if (fabs(solution[i]) < 1e-10) {
                solution[i] = 0.0;  // 自由变量
            } else {
                fprintf(out, "错误: 方程 %d 无解\n", i + 1);
                return 0;
            }
        } else {
            solution[i] /= matrix[i][i];
        }
    }

    return 1;
}

// 解析N元一次方程组
static int parse_linear_system(const char *system_str, double ***matrix_ptr, int *n_ptr, double **constants_ptr, FILE *out) {
    char *str_copy = strdup(system_str);
    if (!str_copy) {
        fprintf(out, "错误: 内存分配失败\n");
        return 0;
    }

    // 分割方程
    char *token;
    char *saveptr;
    int equation_count = 0;
    int max_var_index = 0;  // 最大变量下标

    // 第一遍扫描：统计方程数量和变量数量
    char *temp_str = strdup(system_str);
    char *temp_token = strtok_r(temp_str, ";", &saveptr);

    while (temp_token) {
        equation_count++;

        // 扫描方程中的变量
        char *eq = temp_token;
        char *p = eq;
        while (*p) {
            if ((*p == 'x' || *p == 'X') && isdigit(*(p+1))) {
                int var_idx = atoi(p+1);
                if (var_idx > max_var_index) {
                    max_var_index = var_idx;
                }
                p++; // 跳过x
                while (isdigit(*p)) p++; // 跳过数字
            } else {
                p++;
            }
        }

        temp_token = strtok_r(NULL, ";", &saveptr);
    }
    free(temp_str);

    if (equation_count < 1) {
        fprintf(out, "错误: 未找到有效方程\n");
        free(str_copy);
        return 0;
    }

    if (max_var_index == 0) {
        fprintf(out, "错误: 未找到变量（请使用x1,x2,x3...格式）\n");
        free(str_copy);
        return 0;
    }

    if (equation_count != max_var_index) {
        fprintf(out, "警告: 方程数(%d)不等于变量数(%d)\n", equation_count, max_var_index);
        fprintf(out, "方程组可能无解或有无限多解\n");
    }

    int n = max_var_index;  // 变量个数

    // 分配矩阵内存
    double **matrix = (double **)malloc(equation_count * sizeof(double *));
    double *constants = (double *)calloc(equation_count, sizeof(double));

    for (int i = 0; i < equation_count; i++) {
        matrix[i] = (double *)calloc(n + 1, sizeof(double)); // n个变量+1个常数项
    }

    // 初始化矩阵为0
    for (int i = 0; i < equation_count; i++) {
        for (int j = 0; j <= n; j++) {
            matrix[i][j] = 0.0;
        }
    }

    // 第二遍扫描：解析每个方程
    int eq_idx = 0;
    saveptr = NULL;
    token = strtok_r(str_copy, ";", &saveptr);

    while (token && eq_idx < equation_count) {
        char *eq = token;

        // 去掉首尾空格
        while (*eq == ' ') eq++;
        char *end = eq + strlen(eq) - 1;
        while (end > eq && *end == ' ') *end-- = '\0';

        // 查找等号
        char *equal_sign = strchr(eq, '=');
        if (!equal_sign) {
            fprintf(out, "错误: 方程 %d 缺少等号\n", eq_idx + 1);
            free(str_copy);
            for (int i = 0; i < equation_count; i++) free(matrix[i]);
            free(matrix);
            free(constants);
            return 0;
        }

        *equal_sign = '\0';
        char *left_side = eq;
        char *right_side = equal_sign + 1;

        // 解析右侧常数
        char *endptr;
        double right_const = strtod(right_side, &endptr);
        if (endptr == right_side) {
            right_const = 0.0; // 默认为0
        }
        constants[eq_idx] = right_const;
        matrix[eq_idx][n] = right_const;  // 将常数项放入矩阵最后一列

        // 解析左侧表达式
        char *ptr = left_side;
        double current_coeff = 1.0;
        int is_negative = 0;
        int in_number = 0;
        double number = 0.0;
        int decimal_point = 0;
        double decimal_div = 1.0;

        while (*ptr) {
            // 处理正负号
            if (*ptr == '+' || *ptr == '-') {
                // 保存之前的系数
                if (in_number && !decimal_point) {
                    current_coeff = is_negative ? -number : number;
                }
                is_negative = (*ptr == '-');
                in_number = 0;
                number = 0.0;
                decimal_point = 0;
                decimal_div = 1.0;
                ptr++;
                continue;
            }

            // 处理数字
            if (isdigit(*ptr) || *ptr == '.') {
                in_number = 1;
                if (*ptr == '.') {
                    decimal_point = 1;
                } else {
                    int digit = *ptr - '0';
                    if (decimal_point) {
                        decimal_div *= 10.0;
                        number = number + digit / decimal_div;
                    } else {
                        number = number * 10.0 + digit;
                    }
                }
                ptr++;
                continue;
            }

            // 处理变量
            if (*ptr == 'x' || *ptr == 'X') {
                ptr++; // 跳过'x'

                // 获取变量下标
                int var_idx = 0;
                if (isdigit(*ptr)) {
                    var_idx = atoi(ptr);
                    while (isdigit(*ptr)) ptr++;
                } else {
                    // 简单变量x
                    var_idx = 1;
                }

                // 计算系数
                double coeff = 1.0;
                if (in_number) {
                    coeff = number;
                }
                if (is_negative) {
                    coeff = -coeff;
                }

                if (var_idx > 0 && var_idx <= n) {
                    matrix[eq_idx][var_idx - 1] = coeff;
                } else {
                    fprintf(out, "警告: 方程 %d 中的变量x%d超出范围\n", eq_idx + 1, var_idx);
                }

                // 重置
                current_coeff = 1.0;
                is_negative = 0;
                in_number = 0;
                number = 0.0;
                decimal_point = 0;
                decimal_div = 1.0;
                continue;
            }

            // 跳过空格
            if (*ptr == ' ') {
                ptr++;
                continue;
            }

            // 未知字符
            fprintf(out, "警告: 方程 %d 包含未知字符 '%c'\n", eq_idx + 1, *ptr);
            ptr++;
        }

        // 处理最后一个数字（常数项）
        if (in_number) {
            double constant = is_negative ? -number : number;
            constants[eq_idx] -= constant;
            matrix[eq_idx][n] = constants[eq_idx];
        }

        eq_idx++;
        token = strtok_r(NULL, ";", &saveptr);
    }

    *matrix_ptr = matrix;
    *n_ptr = equation_count;
    *constants_ptr = constants;

    free(str_copy);
    return 1;
}

// 求解N元一次方程组
static int solve_linear_system(const char *system_str, FILE *out) {
    double **matrix = NULL;
    double *constants = NULL;
    int n = 0;

    fprintf(out, "\n=== 高级计算器：N元一次方程组求解 ===\n");

    // 解析方程组
    if (!parse_linear_system(system_str, &matrix, &n, &constants, out)) {
        fprintf(out, "错误: 方程组解析失败\n");
        return 0;
    }

    fprintf(out, "方程组维度: %d 元 %d 个方程\n", n, n);

    // 显示系数矩阵
    fprintf(out, "\n增广矩阵:\n");
    for (int i = 0; i < n; i++) {
        fprintf(out, "[ ");
        for (int j = 0; j < n; j++) {
            fprintf(out, "%8.3f ", matrix[i][j]);
        }
        fprintf(out, "| %8.3f ]\n", constants[i]);
    }

    // 分配解向量内存
    double *solution = (double *)calloc(n, sizeof(double));
    if (!solution) {
        fprintf(out, "错误: 内存分配失败\n");
        for (int i = 0; i < n; i++) free(matrix[i]);
        free(matrix);
        free(constants);
        return 0;
    }

    // 使用高斯消元法求解
    if (!gaussian_elimination(matrix, n, solution, out)) {
        fprintf(out, "错误: 高斯消元法求解失败\n");
        free(solution);
        for (int i = 0; i < n; i++) free(matrix[i]);
        free(matrix);
        free(constants);
        return 0;
    }

    // 输出结果
    fprintf(out, "\n解:\n");
    for (int i = 0; i < n; i++) {
        fprintf(out, "x%d = %12.6f\n", i + 1, solution[i]);
    }

    // 验证解
    fprintf(out, "\n验证（代入原方程）:\n");
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += matrix[i][j] * solution[j];
        }
        fprintf(out, "方程 %d: 计算值 = %8.3f, 实际值 = %8.3f, 误差 = %8.3f\n",
                i + 1, sum, constants[i], fabs(sum - constants[i]));
    }

    fprintf(out, "========================================\n");

    // 释放内存
    free(solution);
    for (int i = 0; i < n; i++) free(matrix[i]);
    free(matrix);
    free(constants);

    return 1;
}

// 主计算器函数
int cmd_calc(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out) {
    (void)ctx;  // 未使用参数
    (void)in;   // 未使用参数

    if (argc < 2) {
        // 显示帮助信息
        fprintf(out, "用法: calc [选项] <表达式>\n");
        fprintf(out, "  科学计算器，支持表达式计算和方程求解\n");
        fprintf(out, "\n");
        fprintf(out, "选项:\n");
        fprintf(out, "  -D          默认模式（数学表达式计算）\n");
        fprintf(out, "  -E          方程模式（一元一次/二次方程）\n");
        fprintf(out, "  -S          线性方程组模式（N元一次方程组）\n");
        fprintf(out, "\n");
        fprintf(out, "默认模式支持的运算:\n");
        fprintf(out, "  + - * /     加减乘除\n");
        fprintf(out, "  ^           幂运算 2^3=8\n");
        fprintf(out, "  [           开方运算 2[4=2 (2次开方)\n");
        fprintf(out, "               [4=2 (默认平方根)\n");
        fprintf(out, "               3[8=2 (3次开方)\n");
        fprintf(out, "  ()          括号\n");
        fprintf(out, "\n");
        fprintf(out, "方程模式支持的格式:\n");
        fprintf(out, "  一元一次方程: ax + b = 0 或 ax = b\n");
        fprintf(out, "  一元二次方程: ax^2 + bx + c = 0\n");
        fprintf(out, "  注意: 系数需明确写出 2x^2+3x-5=0\n");
        fprintf(out, "        支持省略系数: x^2+3x-5=0 (a=1)\n");
        fprintf(out, "        支持负系数: -x^2+3x-5=0\n");
        fprintf(out, "\n");
        fprintf(out, "线性方程组模式（-S）:\n");
        fprintf(out, "  格式: 方程1;方程2;方程3;...\n");
        fprintf(out, "  示例: 2x1+3x2=8;4x1+1x2=6\n");
        fprintf(out, "        1x1+1x2+1x3=6;2x1+1x2+3x3=13;1x1+2x2+1x3=8\n");
        fprintf(out, "  注意: 变量必须为x1,x2,x3,...形式\n");
        fprintf(out, "        系数必须明确写出，即使是1\n");
        fprintf(out, "        方程用分号分隔\n");
        fprintf(out, "\n");
        fprintf(out, "示例:\n");
        fprintf(out, "  calc 2+3 * 4               # 表达式计算\n");
        fprintf(out, "  calc -D 2+3 * 4            # 表达式计算\n");
        fprintf(out, "  calc -E 2x^2+3x-5=0      # 一元二次方程\n");
        fprintf(out, "  calc -E 4x+2=0           # 一元一次方程\n");
        fprintf(out, "  calc -S \"2x1+3x2=8;4x1+1x2=6\"  # 二元一次方程组\n");
        return 0;
    }

    // 处理模式选项
    int mode = 0;  // 0=默认计算模式, 1=方程模式, 2=方程组模式
    int start_arg = 1;

    if (argc >= 2 && argv[1][0] == '-') {
        if (strcmp(argv[1], "-D") == 0) {
            mode = 0;  // 默认模式
            start_arg = 2;
        } else if (strcmp(argv[1], "-E") == 0) {
            mode = 1;  // 方程模式
            start_arg = 2;
        } else if (strcmp(argv[1], "-S") == 0) {
            mode = 2;  // 方程组模式
            start_arg = 2;
        } else {
            fprintf(out, "错误: 未知选项 '%s'\n", argv[1]);
            fprintf(out, "使用 'calc' 查看帮助\n");
            return 1;
        }
    }

    if (start_arg >= argc) {
        fprintf(out, "错误: 缺少表达式或方程\n");
        fprintf(out, "使用 'calc' 查看帮助\n");
        return 1;
    }

    // 合并参数为单个字符串
    char expression[1024] = "";
    for (int i = start_arg; i < argc; i++) {
        if (i > start_arg && expression[0] != '\0') {
            strcat(expression, " ");
        }
        strncat(expression, argv[i], sizeof(expression) - strlen(expression) - 1);
    }

    if (mode == 0) {
        // 默认计算模式
        fprintf(out, "表达式: %s\n", expression);
        double result = evaluate_expression(expression);

        // 检查是否为整数
        if (fabs(result - round(result)) < 1e-10) {
            fprintf(out, "结果: %.0f\n", result);
        } else {
            fprintf(out, "结果: %.10f\n", result);
        }
    } else if (mode == 1) {
        // 方程模式
        fprintf(out, "\n[高级计算器] 方程模式\n");
        fprintf(out, "注意: 只支持一元实系数方程\n");
        fprintf(out, "支持方程格式:\n");
        fprintf(out, "  一元一次: ax + b = 0 或 ax = b\n");
        fprintf(out, "  一元二次: ax^2 + bx + c = 0\n");
        fprintf(out, "示例: 2x^2+3x-5=0, 4x+2=0, x^2-4=0\n\n");

        double a = 0.0, b = 0.0, c = 0.0;

        if (!parse_equation_simple(expression, &a, &b, &c, out)) {
            fprintf(out, "错误: 无法解析方程 '%s'\n", expression);
            fprintf(out, "请检查方程格式是否正确\n");
            fprintf(out, "正确格式: ax^2 + bx + c = 0 或 ax + b = 0\n");
            fprintf(out, "         (可以省略系数，x^2表示x平方)\n");
            return 1;
        }

        fprintf(out, "系数: a=%.6f, b=%.6f, c=%.6f\n", a, b, c);

        // 判断是否二次方程
        int is_quadratic = 0;
        char eq_lower[256];

        // 转换为小写
        for (int i = 0; expression[i] && i < 255; i++) {
            eq_lower[i] = tolower(expression[i]);
        }
        eq_lower[255] = '\0';

        // 是否包含x^2
        if (strstr(eq_lower, "x^2") != NULL) {
            is_quadratic = 1;
        } else if (fabs(a) > 1e-10) {
            // a系数不为0也是二次方程
            is_quadratic = 1;
        }

        if (is_quadratic) {
            solve_quadratic_equation(a, b, c, out);
        } else {
            solve_linear_equation(b, c, out);
        }
    } else if (mode == 2) {
        // 方程组模式
        solve_linear_system(expression, out);
    }

    return 0;
}
