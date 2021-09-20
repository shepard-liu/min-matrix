///////////////////////////////////////////////////////////////////////////////////
//###############################################################################//
//#									 Matrix.h
//#								     矩阵类模板
//#
//#	    <类>		        <描述>		<关系>		<描述>
//#	    Matrix<T>			矩阵类	    基类	    具有线性代数中矩阵的基本计算功能
//#	    Determinant<T>		行列式类	包含矩阵类	  包含一个矩阵类指针，具有求值功能
//#
//#     矩阵元素访问函数 operator()() 行列序号默认从1开始，若要使用0作为序号起始，请在包含本
//# 头文件前定义宏 MATRIX_INDEX_START_AT_0:
//#
//#      #define MATRIX_INDEX_START_AT_0
//#      
//#     矩阵的数据以行存储（Row Major）
//#																	2020/08/05
//#																	Shepard Liu
//#								   Version:0.1
//#
//#     加入了Doxygen注释格式
//#     修正了单位矩阵的函数,抽象更合理
//#     添加简单错误处理
//#                                                                 2021/05/11
//#																	Shepard Liu
//#                                Version:0.2
//#
//#
//#     移除了单位矩阵类，现在由Matrix<T>的静态成员函数进行构造
//#     增加了两个矩阵的构造函数，现在可以使用类MATLAB语法和初始化列表进行矩阵构造
//#     加入特殊矩阵生成函数
//#     提供下标从0开始的重载版本元素访问函数 operator()() 和 ElemAt0()，可通过定义宏
//# MATRIX_INDEX_START_AT_0 来启用下标为0版本的 operator()() 因此注意后续开发中
//# 在Matrix.h中禁止使用 operator()()
//#                                                                 2021/09/16
//#																	Shepard Liu
//#                                Version:0.3
//#
//#
//###############################################################################//
///////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <cassert>
#include <vector>
#include <initializer_list>
#include <sstream>
#include <random>
#include <chrono>

template <typename T, size_t _CapacityIncrement = 2>
class Matrix;

typedef Matrix<double> Matrixd;

template <typename T>
class Determinant;

/**
    @brief 矩阵类
    
    矩阵基类，提供基础的矩阵运算与输入输出功能

    @tparam T	                矩阵数据类型
    @tparam _CapacityIncrement	矩阵扩增系数
*/
template <typename T, size_t _CapacityIncrement>
class Matrix
{
    //声明
    friend Determinant<T>;

protected:
    size_t uRow; //行数
    size_t uCol; //列数

    T *pData = nullptr; //矩阵数据
    size_t uCapacity;   //数据容量
private:
    static const size_t uCapacityIncrement = _CapacityIncrement > 1 ? _CapacityIncrement : 2; //容量倍增系数

public:
    enum Direction
    {
        LEFT,
        RIGHT,
        ABOVE,
        BELOW,
        TOPLEFT,
        TOPRIGHT,
        BOTLEFT,
        BOTRIGHT,
    };

public:
    //构造函数

    /**
     * @brief   无参构造函数
     * 
     * 构造0行0列无数据的空矩阵
     */
    Matrix() : Matrix(0, 0)
    {
    }

    /**
        @brief 无数据构造函数：

        构造一个row × col大小的矩阵
        @param row 矩阵的行数
        @param col 矩阵的列数
    */
    Matrix(size_t row, size_t col) : uRow(row), uCol(col)
    {
        uCapacity = uCapacityIncrement * row * col;
        pData = new T[uCapacity]{};
        assert(pData);
    }

    /**
        @brief 矩阵构造函数：指定所有元素为某值

        构造一个row × col大小的，元素全为同一值的矩阵
        @param row 矩阵的行数
        @param col 矩阵的列数
    */
    Matrix(size_t row, size_t col, const T &value) : uRow(row), uCol(col)
    {
        uCapacity = uCapacityIncrement * row * col;
        pData = new T[uCapacity]{};
        for (size_t i = 0; i < row * col; ++i)
            pData[i] = value;
        assert(pData);
    }

    /**
        @brief 矩阵构造函数：数组指针

        构造一个row × col大小的矩阵，复制线性数组data的数据
        @param row	矩阵的行数
        @param col	矩阵的列数
        @param data	矩阵数据
        @param dataLen  数组元素个数
    */
    Matrix(size_t row, size_t col, const T *data, size_t dataLen) : Matrix(row, col)
    {
        size_t elemCount = row * col;
        memcpy_s(pData, sizeof(T) * elemCount, data,
                 dataLen > elemCount ? elemCount * sizeof(T) : dataLen * sizeof(T));
    }

    /**
     * @brief 矩阵构造函数：从字符串
     * 
     * 使用类似于MATLAB中使用的矩阵定义字符串创建矩阵对象.
     * 例如："[ 1 2, 5; 2.5 0 -1.3e2; 3, 2 6 ]"将转换为矩阵
     * 
     * 1    2   5
     * 2.5  0   -1.3e2
     * 3    2   6
     * 
     * 按行指定，每个元素间用分隔符' '或','且多余的空格自动忽略
     * 该函数将检查表达式是否合法，第一行元素个数决定矩阵有多少列，
     * 若后续行中缺项，将报错。
     * 
     * @param expr 类MATLAB的矩阵定义字符串
     */
    explicit Matrix(const std::string &expr)
    {
        //错误标识打印
        auto flagPrint = [](const std::string &str, size_t flagPos)
        {
            std::cout << '\n'
                      << str << '\n';
            for (size_t i = 0; i < flagPos; ++i)
                std::cout << ' ';
            std::cout << "^\n";
        };

        //转换矩阵行：传入rowStr需保证没有'[', ';', ']'
        auto parseRow = [&](const std::string &rowStr, int &errIdx)
        {
            std::string str = ' ' + rowStr + ' ';

            std::vector<T> values;
            size_t numStartIdx = 0;
            bool inNumber = false;

            for (size_t i = 0; i < str.size(); ++i)
            {
                if (str[i] != ' ' && str[i] != ',' && !inNumber)
                {
                    numStartIdx = i;
                    inNumber = true;
                }
                if (inNumber && (str[i] == ' ' || str[i] == ','))
                {
                    inNumber = false;
                    std::stringstream ss(str.substr(numStartIdx, i - numStartIdx));
                    T num = T(0);
                    auto &is = (ss >> num);
                    if (is.fail() || !is.eof())
                    {
                        errIdx = numStartIdx;
                        return std::vector<T>();
                    }
                    values.push_back(num);
                }
            }

            errIdx = -1;
            return values;
        };

        //开始转换矩阵字符串

        size_t lbraIdx = expr.find('[');
        //保证开始符‘[’前不存在除空格以外的符号
        for (size_t i = 0; i < lbraIdx; ++i)
        {
            if (expr[i] != ' ')
            {
                std::cout << "Error parsing matrix expression (unrecognized character before '['):\n";
                flagPrint(expr, i);
                assert(0);
            }
        }

        size_t rbraIdx = expr.find(']');
        //保证结束符']'后不存在除空格以外的符号
        for (size_t i = rbraIdx + 1; i < expr.size(); ++i)
        {
            if (expr[i] != ' ')
            {
                std::cout << "Error parsing matrix expression (unrecognized character after ']'):\n";
                flagPrint(expr, i);
                assert(0);
            }
        }

        ////////////逐行字符检查

        std::vector<std::vector<T>> dataVec;

        //检查第一行
        size_t rowDelimIdx = expr.find(';');
        if (rowDelimIdx == std::string::npos)
            rowDelimIdx = rbraIdx;

        int errIdx = -1;
        dataVec.push_back(parseRow(expr.substr(lbraIdx + 1, rowDelimIdx - lbraIdx - 1), errIdx));
        if (errIdx != -1)
        {
            std::cout << "Error parsing matrix expression (unable to parse row element):\n";
            flagPrint(expr, errIdx + lbraIdx);
            assert(0);
        }

        uCol = dataVec.front().size();

        //检查接下来的行

        bool onEnd = false;

        while (!onEnd)
        {
            size_t rowNextDelimIdx = expr.find(';', rowDelimIdx + 1);
            if (rowNextDelimIdx == std::string::npos)
            {
                rowNextDelimIdx = rbraIdx;
                onEnd = true;
            }

            dataVec.push_back(parseRow(expr.substr(rowDelimIdx + 1, rowNextDelimIdx - rowDelimIdx - 1), errIdx));
            if (errIdx != -1)
            {
                std::cout << "Error parsing matrix expression (unable to parse row element):\n";
                flagPrint(expr, errIdx + rowDelimIdx);
                assert(0);
            }

            //保证列数相同
            if (dataVec.back().size() != uCol)
            {
                std::cout << "Error parsing matrix expression (matrix rows must have consistent number of elements):\n";
                flagPrint(expr, rowNextDelimIdx);
                assert(0);
            }

            rowDelimIdx = rowNextDelimIdx;
        }

        //确定行数，输入数据
        uRow = dataVec.size();
        pData = new T[uCol * uRow];
        for (size_t i = 0; i < dataVec.size(); ++i)
            memcpy_s(pData + i * uCol, uCol * sizeof(T), dataVec[i].data(), uCol * sizeof(T));
    }

    /**
     * @brief 矩阵构造函数：从初始化列表
     * 
     * 使用嵌套的初始化列表构造矩阵
     * 格式如 { {2, 5, 9}, {1.0, 22, 6}, {0, 98, 6}, ...}
     * 每个内列表指定行元素，第一个内列表的元素个数指定矩阵列数
     * 若后续内列表缺项，将报错。
     * 
     * @param iList 嵌套的初始化列表
     */
    explicit Matrix(std::initializer_list<std::initializer_list<T>> iList)
    {
        uCol = (*iList.begin()).size();
        uRow = iList.size();
        pData = new T[uCol * uRow];

        for (size_t i = 0; i < uRow; ++i)
        {
            auto &inner = iList.begin()[i];
            assert(uCol == inner.size());
            memcpy_s(pData + i * uCol, uCol * sizeof(T), inner.begin(), uCol * sizeof(T));
        }
    }

    /**
     * @brief  矩阵构造函数：从变长数组
     * 
     * @param row 行数
     * @param col 列数
     * @param vec 数据
     */
    Matrix(size_t row, size_t col, const std::vector<T> &vec) : Matrix(row, col, vec.data(), vec.size()) {}

    /**
        @brief  拷贝构造函数：

        使用现有矩阵复制构造
        @param mat 矩阵对象的引用
    */
    Matrix(const Matrix &mat) : Matrix(mat.uRow, mat.uCol, mat.pData, mat.uRow * mat.uCol) {}

    /**
        @brief  移动拷贝构造函数：
        右值引用构造
        @param mat 矩阵对象的右值引用，其数据指针会被置空
    */
    Matrix(Matrix<T> &&mat) noexcept : uRow(mat.uRow), uCol(mat.uCol), pData(mat.pData), uCapacity(mat.uCapacity)
    {
        mat.pData = nullptr;
    }

    /**
        @brief  赋值运算符函数

        @param mat 矩阵对象的引用
        @return 返回被赋值对象的引用
    */
    Matrix<T> &operator=(const Matrix<T> &mat)
    {
        if (this == &mat)
            return *this;
        this->uRow = mat.uRow;
        this->uCol = mat.uCol;
        this->pData = new T[mat.uCapacity];
        assert(pData);
        memcpy_s(this->pData, this->uRow * this->uCol * sizeof(T), mat.pData, mat.uRow * mat.uCol * sizeof(T));
        return *this;
    }

    /**
        @brief  移动赋值运算符函数：

        使用右值移动赋值
        @param mat 矩阵对象的右值引用，其数据指针会被置空
        @return 被赋值对象的引用
    */
    Matrix<T> &operator=(Matrix<T> &&mat) noexcept
    {
        if (&mat == this)
            return *this;
        this->uRow = mat.uRow;
        this->uCol = mat.uCol;
        this->uCapacity = mat.uCapacity;
        this->pData = mat.pData;
        mat.pData = nullptr;
        return *this;
    }

    //析构函数
    virtual ~Matrix()
    {
        if (pData != nullptr)
            delete[] pData;
    }

public:
    //矩阵输出
    template <typename E>
    friend std::ostream &operator<<(std::ostream &os, const Matrix<E> &mat);

public:
    /**
        获取矩阵的数据
        @return 矩阵数据的头指针
    */
    inline T *Data()
    {
        return this->pData;
    }

public:
    /**
        获取矩阵的行数
        @return 矩阵的行数
    */
    size_t RowSize() const
    {
        return uRow;
    }

public:
    /**
        获取矩阵的列数
        @return 矩阵的列数
    */
    size_t ColumnSize() const
    {
        return uCol;
    }

private:
    //数据扩增
    void Expand()
    {
        T *pOldData = pData;
        pData = new T[uCapacityIncrement * uCapacity];
        assert(pData);
        memcpy_s(pData, sizeof(T) * uCapacity, pOldData, sizeof(T) * uCapacity);
        uCapacity *= uCapacityIncrement;
        delete[] pOldData;
    }

protected:
    /**
     * @brief 访问矩阵元素，下标从1开始。
     * 
     * @param row 行号，从1开始
     * @param col 列号，从1开始
     * @return const T& 元素的常引用
     */
    inline const T &ElementAt(size_t row, size_t col) const
    {
        return pData[(row - 1) * uCol + col - 1];
    }

public:
#ifdef MATRIX_INDEX_START_AT_0
    /**
     * @brief 获取矩阵元素的引用，下标从0开始。
     * 
     * 因为已经定义 MATRIX_INDEX_START_AT_0 ,本函数启用。若要
     * 使该函数下标从1开始，请勿定义 MATRIX_INDEX_START_AT_0
     * 
     * @param row 行号，从0开始
     * @param col 列号，从0开始
     * @return T& 元素的引用
     */
    inline T &operator()(size_t row, size_t col)
    {
        assert(row < uRow && col < uCol);
        return pData[row * uCol + col];
    }
#else
    /**
     * @brief 获取矩阵元素的引用，行列序号从1开始。
     * 
     * 因为未定义 MATRIX_INDEX_START_AT_0 ,本函数启用。若要
     * 使该函数行列序号从0开始，请在包含本头文件前定义 MATRIX_INDEX_START_AT_0
     * 
     * @param row 行号，从1开始
     * @param col 列号，从1开始
     * @return T& 元素的引用
     */
    inline T &operator()(size_t row, size_t col)
    {
        assert(row > 0 && col > 0 && row <= uRow && col <= uCol);
        return pData[(row - 1) * uCol + col - 1];
    }
#endif

public:
    /**
        @brief 元素访问与修改，行列序号从1开始

        该函数的行列序号始终从1开始，无论 MATRIX_INDEX_START_AT_0
        是否被定义

        @param row 元素所在的行数，从1开始
        @param col 元素所在的列数，从1开始
        @return 矩阵元素的引用
    */
    inline T &ElemAt(size_t row, size_t col)
    {
        assert(row > 0 && col > 0 && row <= uRow && col <= uCol);
        return pData[(row - 1) * uCol + col - 1];
    }

public:
    /**
        @brief 元素访问与修改，行列序号从1开始

        该函数的行列序号始终从1开始，无论 MATRIX_INDEX_START_AT_0
        是否被定义

        @param row 元素所在的行数，从1开始
        @param col 元素所在的列数，从1开始
        @return 矩阵元素的引用
    */
    inline const T &ElemAt(size_t row, size_t col) const
    {
        assert(row > 0 && col > 0 && row <= uRow && col <= uCol);
        return pData[(row - 1) * uCol + col - 1];
    }

public:
    /**
        @brief 元素访问与修改，行列序号从0开始

        该函数的行列序号始终从0开始，无论 MATRIX_INDEX_START_AT_0
        是否被定义

        @param row 元素所在的行数，从0开始
        @param col 元素所在的列数，从0开始
        @return 矩阵元素的引用
    */
    inline T &ElemAt0(size_t row, size_t col)
    {
        assert(row < uRow && col < uCol);
        return pData[row * uCol + col];
    }

public:
    /**
        @brief 元素访问与修改，行列序号从0开始

        该函数的行列序号始终从0开始，无论 MATRIX_INDEX_START_AT_0
        是否被定义

        @param row 元素所在的行数，从0开始
        @param col 元素所在的列数，从0开始
        @return 矩阵元素的引用
    */
    inline const T &ElemAt0(size_t row, size_t col) const
    {
        assert(row < uRow && col < uCol);
        return pData[row * uCol + col];
    }

private:
    /**
     * @brief 求元素在线性数组中的下标
     * 
     * @param row 行号，从1开始
     * @param col 列号，从1开始
     * @return size_t 元素在数据中的下标
     */
    inline size_t IndexAt(size_t row, size_t col) const
    {
        return (row - 1) * uCol + col - 1;
    }

public:
    /**
     * @brief 遍历矩阵元素
     * 
     * @param pOps 对每个元素执行的操作
     * @return Matrix<T>& 本对象的引用
     */
    Matrix<T> &ForEach(bool (*pOps)(T &))
    {
        auto ps = pData - 1;
        auto pe = pData + uCol * uRow;
        while (ps != pe)
            if (!pOps(*(++ps)))
                break;
        return *this;
    }

public:
    /**
     * @brief 遍历矩阵元素
     * 
     * @param pOps 对每个元素执行的操作(函数指针)
     * @return Matrix<T>& 本对象的引用
     */
    Matrix<T> &ForEach(bool (*pOps)(T &, size_t))
    {
        auto ps = pData - 1;
        auto pe = pData + uCol * uRow;
#ifdef MATRIX_INDEX_START_AT_0
        size_t idx = -1;
#else
        size_t idx = 0;
#endif
        while (ps != pe)
            if (!pOps(*(++ps), ++idx))
                break;
        return *this;
    }

public:
    /**
        @brief 在矩阵中插入一行数据，可能会引起数据扩增

        若定义了MATRIX_INDEX_START_AT_0,则行列序号从0开始，
        否则从1开始。

        @param pos			要插入到的行序号
        @param pNewRowData	要插入的行数据的指针
        @param dataSize		要插入的行数据的个数，若少于列数，将使用0补齐
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &InsertRow(size_t pos, const T *pNewRowData, size_t dataSize)
    {
#ifdef MATRIX_INDEX_START_AT_0
        ++pos;
#endif
        //检查位置是否合法
        assert(pos <= uRow + 1 && pos != 0);

        //实际添加的数据个数n
        size_t n = uCol < dataSize ? uCol : dataSize;

        //检查容量是否充足，否则分配内存
        if ((uRow + 1) * uCol < uCapacity)
            Expand();

        //向后移动数据
        //获取末尾元素指针
        T *pLastElem = &ElemAt(uRow, uCol);
        size_t CopyLen = 1 + IndexAt(uRow, uCol) - IndexAt(pos, 1);
        for (size_t i = 0; i < CopyLen; ++i)
            *(pLastElem - i + uCol) = *(pLastElem - i);

        ++uRow;
        T *pNewRowHead = &ElemAt(pos, 1);
        for (size_t i = 0; i < n; ++i) //复制数据
            *(pNewRowHead + i) = pNewRowData[i];
        for (size_t i = n; i < uCol; ++i) //用0把这行填满
            *(pNewRowHead + i) = T(0);

        return *this;
    }

public:
    /**
        @brief 在矩阵中插入一行数据，可能会引起数据扩增

        若定义了MATRIX_INDEX_START_AT_0,则行列序号从0开始，
        否则从1开始。

        @param pos			要插入到的行序号
        @param pNewRowData	要插入的行数据的指针
        @param dataSize		要插入的行数据的个数，若少于列数，将使用0补齐
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &InsertRow(size_t pos, const std::vector<T> &rowData)
    {
        return InsertRow(pos, rowData.data(), rowData.size());
    }

public:
    /**
        @brief 在矩阵中插入一列数据，可能会引起数据扩增

        若定义了MATRIX_INDEX_START_AT_0,则行列序号从0开始，
        否则从1开始。

        @param pos			要插入到的列序号
        @param pNewRowData	要插入的列数据的指针
        @param dataSize		要插入的列数据的个数，若少于行数，将使用0补齐
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &InsertColumn(size_t pos, const T *pNewColData, size_t dataSize)
    {
#ifdef MATRIX_INDEX_START_AT_0
        ++pos;
#endif
        //检查插入位置是否合法
        assert(pos <= uCol + 1 && pos != 0);

        //实际添加的数据个数n
        size_t n = uRow < dataSize ? uRow : dataSize;

        //检查容量是否充足，否则分配内存
        if (uRow * (uCol + 1) < uCapacity)
            Expand();

        //向后移动数据--------------
        //获取末尾元素指针
        T *pLastElem = &ElemAt(uRow, uCol);
        //先处理最后一行在插入列右侧的数据的移动
        for (size_t i = 0; i < uCol - pos + 1; ++i)
            *(pLastElem - i + uRow) = *(pLastElem - i);
        //再向前分每uCol个元素一组处理数据移动
        for (size_t Shift = uRow - 1; Shift >= 1; --Shift)
        {
            T *pSectionLast = &ElemAt(Shift + 1, pos - 1);
            for (size_t i = 0; i < uCol; ++i)
                *(pSectionLast - i + Shift) = *(pSectionLast - i);
        }

        //拷贝数据到新列----------
        ++uCol;
        //获取空出的第一个位置的指针
        T *pBlankPos = &ElemAt(1, pos);
        for (size_t i = 0; i < n; ++i)
            *(pBlankPos + i * uCol) = pNewColData[i];
        for (size_t i = n; i < uRow; ++i)
            *(pBlankPos + i * uCol) = T(0);

        return *this;
    }

public:
    /**
        @brief 在矩阵中插入一列数据，可能会引起数据扩增

        若定义了MATRIX_INDEX_START_AT_0,则行列序号从0开始，
        否则从1开始。

        @param pos		要插入到的列序号
        @param colData	要插入的列数据
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &InsertColumn(size_t pos, const std::vector<T> &colData)
    {
        return InsertColumn(pos, colData.data(), colData.size());
    }

public:
    /**
        @brief 矩阵最下方添加一行数据，可能会引起数据扩增

        @param pNewRowData	要添加的行数据的指针
        @param dataSize		要添加的行数据的个数，若少于列数，将使用0补齐
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &AddRow(const T *pNewRowData, size_t dataSize)
    {
#ifdef MATRIX_INDEX_START_AT_0
        return InsertRow(uRow, pNewRowData, dataSize);
#else
        return InsertRow(uRow + 1, pNewRowData, dataSize);
#endif
    }

public:
    /**
        @brief 矩阵最下方添加一行数据，可能会引起数据扩增

        @param rowData	要添加的行数据
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &AddRow(const std::vector<T> &rowData)
    {
#ifdef MATRIX_INDEX_START_AT_0
        return InsertRow(uRow, rowData.data(), rowData.size());
#else
        return InsertRow(uRow + 1, rowData.data(), rowData.size());
#endif
    }

public:
    /**
        @brief 矩阵最右侧添加一列数据，可能会引起数据扩增

        @param pNewColData	要添加的列数据的指针
        @param dataSize		要添加的列数据的个数，若少于行数，将使用0补齐
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &AddColumn(const T *pNewColData, size_t dataSize)
    {
#ifdef MATRIX_INDEX_START_AT_0
        return InsertColumn(uCol, pNewColData, dataSize);
#else
        return InsertColumn(uCol + 1, pNewColData, dataSize);
#endif
    }

public:
    /**
        @brief 矩阵最右侧添加一列数据，可能会引起数据扩增

        @param colData	要添加的列数据的指针
        @return Matrix<T>&  本对象的引用
    */
    Matrix<T> &AddColumn(const std::vector<T> &colData)
    {
#ifdef MATRIX_INDEX_START_AT_0
        return InsertColumn(uCol, colData.data(), colData.size());
#else
        return InsertColumn(uCol + 1, colData.data(), colData.size());
#endif
    }

public:
    /**
     * @brief 获取矩阵区块
     * 
     *  若定义了MATRIX_INDEX_START_AT_0,则行列序号从0开始，
     *  否则从1开始。
     * 
     * @param rowStart      区块行起始序号
     * @param colStart      区块列起始序号
     * @param rowSpan       区块行数
     * @param colSpan       区块列数
     * @return Matrix<T>    区块拷贝得到的矩阵
     */
    Matrix<T> Block(size_t rowStart, size_t colStart, size_t rowSpan, size_t colSpan)
    {
        //这个V0.3新加的函数用0为序号基准
#ifndef MATRIX_INDEX_START_AT_0
        --rowStart;
        --colStart;
#endif
        assert(rowStart > 0 && rowStart <= uRow && colStart > 0 && colStart <= uCol);
        rowSpan = rowSpan + rowStart > uRow ? uRow - rowStart : rowSpan;
        colSpan = colSpan + colStart > uCol ? uCol - colStart : colSpan;

        Matrix<T> blockMat(rowSpan, colSpan);
        for (size_t i = 0; i < rowSpan; ++i)
            for (size_t j = 0; j < colSpan; ++j)
                blockMat.ElemAt0(i, j) = this->ElemAt0(i + rowStart, j + colStart);

        return blockMat;
    }

public:
    /**
        @brief 将该矩阵与参数中的矩阵合并

        LEFT	    将mat置于左侧合并，要求行数相同
        RIGHT	    将mat置于右侧合并，要求行数相同
        ABOVE	    将mat置于上方合并，要求列数相同
        BELOW	    将mat置于下方合并，要求列数相同
        TOPLEFT	    将mat置于左上角合并，其余位置补0
        TOPRIGHT    将mat置于右上角合并，其余位置补0
        BOTLEFT	    将mat置于左下角合并，其余位置补0
        BOTRIGHT	将mat置于右下角合并，其余位置补0

        @param mat	要合并的第二个矩阵
        @param d	第二个矩阵的相对位置

        @return				合并的矩阵
    */
    Matrix<T> CombineWith(const Matrix<T> &mat, Direction d) const
    {
        switch (d)
        {
        case LEFT:
        {
            // 检查行数是否相同
            assert(this->uRow == mat.uRow);

            Matrix<T> r(uRow, this->uCol + mat.uCol);

            T *pRowHead = r.pData;
            T *pSubRowHead1 = mat.pData;
            T *pSubRowHead2 = this->pData;

            for (size_t i = 1; i <= uRow; ++i)
            {
                //对每一行先复制mat再复制this
                for (size_t j = 0; j < mat.uCol; ++j)
                    pRowHead[j] = pSubRowHead1[j];
                pRowHead += mat.uCol;
                pSubRowHead1 += mat.uCol;
                for (size_t j = 0; j < this->uCol; ++j)
                    pRowHead[j] = pSubRowHead2[j];
                pRowHead += this->uCol;
                pSubRowHead2 += this->uCol;
            }
            return r;
        }
        case RIGHT:
            return mat.CombineWith(*this, LEFT);

        case ABOVE:
        {
            //检查列数是否相同
            assert(this->uCol == mat.uCol);

            Matrix<T> r(this->uRow + mat.uRow, uCol);
            T *pHead = r.pData;

            for (size_t i = 0; i < mat.uRow * mat.uCol; ++i)
                pHead[i] = mat.pData[i];
            pHead += mat.uRow * mat.uCol;
            for (size_t i = 0; i < uRow * uCol; ++i)
                pHead[i] = this->pData[i];

            return r;
        }
        case BELOW:
            return mat.CombineWith(*this, ABOVE);

        case TOPLEFT:
        {
            Matrix<T> r(this->uRow + mat.uRow, this->uCol + mat.uCol);
            T *pHead = r.pData;
            T *pSubHead1 = mat.pData;
            T *pSubHead2 = this->pData;

            for (size_t i = 1; i <= mat.uRow; ++i)
            {
                for (size_t j = 0; j < mat.uCol; ++j)
                    pHead[j] = pSubHead1[j];
                pHead += r.uCol;
                pSubHead1 += mat.uCol;
            }

            pHead += mat.uCol;

            for (size_t i = 1; i <= this->uRow; ++i)
            {
                for (size_t j = 0; j < this->uCol; ++j)
                    pHead[j] = pSubHead2[j];
                pHead += r.uCol;
                pSubHead2 += this->uCol;
            }

            return r;
        }

        case TOPRIGHT:
        {
            Matrix<T> r(this->uRow + mat.uRow, this->uCol + mat.uCol);
            T *pHead = r.pData + this->uCol;
            T *pSubHead1 = mat.pData;
            T *pSubHead2 = this->pData;

            for (size_t i = 1; i <= mat.uRow; ++i)
            {
                for (size_t j = 0; j < mat.uCol; ++j)
                    pHead[j] = pSubHead1[j];
                pHead += r.uCol;
                pSubHead1 += mat.uCol;
            }

            pHead += mat.uCol;

            for (size_t i = 1; i <= this->uRow; ++i)
            {
                for (size_t j = 0; j < this->uCol; ++j)
                    pHead[j] = pSubHead2[j];
                pHead += r.uCol;
                pSubHead2 += this->uCol;
            }

            return r;
        }

        case BOTLEFT:
            return mat.CombineWith(*this, TOPRIGHT);

        case BOTRIGHT:
            return mat.CombineWith(*this, TOPLEFT);

        default:
            assert(0);
        }
        return Matrix<T>();
    }

public:
    /**
     *  @brief 矩阵按行分解，保留指定方向的矩阵
     *  
     *  取值：
     *  ABOVE	保留分割行和其上方的部分
     *  BELOW	保留分割行和其下方的部分
     * 
     *  若定义了MATRIX_INDEX_START_AT_0,则行列序号从0开始，
     *  否则从1开始。
     * 
     *  @param SplitterRowIndex		分割行的序号，该行将被保留在返回的矩阵中
     *  @param d			        分割保留的方向
     *   
     *  @return		分割后保留的部分构成的矩阵
     */
    Matrix<T> RowSplit(size_t SplitterRowIndex, Direction d) const
    {
        //n：结果矩阵的行数
        size_t n = SplitterRowIndex <= this->uRow ? SplitterRowIndex : this->uRow;

        if (d == ABOVE)
        {
            Matrix<T> r(n, this->uCol);
            memcpy_s(r.pData, r.uRow * r.uCol * sizeof(T), this->pData, r.uRow * r.uCol * sizeof(T));
            return r;
        }
        else if (d == BELOW)
        {
            Matrix<T> r(this->uRow - n + 1, this, uCol);
            memcpy_s(r.pData, r.uRow * r.uCol * sizeof(T), &this->ElemAt(n, 1), r.uRow * r.uCol * sizeof(T));
            return r;
        }

        assert(0);
        return Matrix<T>();
    }

public:
    /**
     *  @brief 矩阵按列分解，保留指定方向的矩阵
     * 
     *  取值：
     *  ON_LFET		0	保留分割列和其左边的部分
     *  ON_RIGHT	1	保留分割列和其右边的部分
     *   
     *  若定义了MATRIX_INDEX_START_AT_0,则行列序号从0开始，
     *  否则从1开始。
     *
     *  @param SplitterRowIndex		分割列的序号，该列将被保留在返回的矩阵中
     *  @param d			分割保留的方向
     *   
     *  @return		分割后保留的部分构成的矩阵
     */
    Matrix<T> ColumnSplit(size_t SplitterColIndex, Direction d) const
    {
        //n：结果矩阵的列数
        size_t n = SplitterColIndex <= this->uCol ? SplitterColIndex : this->uCol;

        if (d == LEFT)
        {
            Matrix<T> r(uRow, n);
            T *pSubRowHead = r.pData;
            T *pRowHead = this->pData;

            for (size_t i = 1; i <= uRow; ++i)
            {
                for (size_t j = 0; j < n; ++j)
                    pSubRowHead[j] = pRowHead[j];
                pSubRowHead += r.uCol;
                pRowHead += this->uCol;
            }
            return r;
        }
        else if (d == RIGHT)
        {
            Matrix<T> r(uRow, this->uCol - n + 1);
            T *pSubRowHead = r.pData;
            T *pRowHead = this->pData + n - 1;

            for (size_t i = 1; i <= uRow; ++i)
            {
                for (size_t j = 0; j < n; ++j)
                    pSubRowHead[j] = pRowHead[j];
                pSubRowHead += r.uCol;
                pRowHead += this->uCol;
            }
            return r;
        }

        assert(0);
        return Matrix<T>();
    }

public:
    /**
        @brief 矩阵同型检查：检查矩阵1和矩阵2是否为同型矩阵

        @param mat1		矩阵1
        @param mat2		矩阵2
        @return		若mat1和mat2为同型矩阵，则返回true，否则返回false
    */
    static bool Varify_Homo(const Matrix<T> &mat1, const Matrix<T> &mat2)
    {
        if ((mat1.uRow - mat2.uRow) || (mat1.uCol - mat2.uCol)) //如果行数列数不等
            return false;
        return true;
    }

public:
    //矩阵取负
    Matrix<T> operator-() const
    {
        Matrix<T> resMat(uRow, uCol);
        T *pResData = resMat.pData - 1;
        T *pThisData = pData - 1;
        for (size_t i = 0; i < uRow * uCol; ++i)
            *(++pResData) = -*(++pThisData);
        return resMat;
    }

public:
    //矩阵加法
    Matrix<T> operator+(const Matrix<T> &mat) const
    {
        //同型检查
        assert(Varify_Homo(*this, mat));

        Matrix<T> r(uRow, uCol);
        for (size_t i = 0; i < uRow * uCol; ++i)
            r.pData[i] = this->pData[i] + mat.pData[i];
        return r;
    }

public:
    //矩阵减法
    Matrix<T> operator-(const Matrix<T> &mat) const
    {
        //同型检查
        assert(Varify_Homo(*this, mat));

        Matrix<T> r(uRow, uCol);
        for (size_t i = 0; i < uRow * uCol; ++i)
            r.pData[i] = this->pData[i] - mat.pData[i];
        return r;
    }

public:
    //矩阵数乘（数在右）
    Matrix<T> operator*(const T &c) const
    {
        Matrix<T> r(uRow, uCol);
        for (size_t i = 0; i < uRow * uCol; ++i)
            r.pData[i] = this->pData[i] * c;
        return r;
    }

public:
    //矩阵数乘（数在左）
    template <typename E>
    friend Matrix<E> operator*(const T &c, const Matrix<E> &mat);

public:
    //矩阵点乘
    Matrix<T> operator*(const Matrix<T> &mat) const
    {
        assert(this->uCol == mat.uRow);

        Matrix<T> r(this->uRow, mat.uCol);
        for (size_t i = 1; i <= r.uRow; ++i)
            for (size_t j = 1; j <= r.uCol; ++j)
                for (size_t k = 1; k <= this->uCol; ++k)
                    r.ElemAt(i, j) += this->ElementAt(i, k) * mat.ElementAt(k, j);
        return r;
    }

private:
    /**
        @brief 矩阵初等变换：行交换：交换两行的数据。

        使用初等变换函数相比左乘变换方阵更加高效

        若定义了MATRIX_INDEX_START_AT_0,则行序号从0开始，
        否则从1开始。

        @param RowIndex1	第一个行序号
        @param RowIndex2	第二个行序号
    */
    void RowInterchange(size_t RowIndex1, size_t RowIndex2)
    {
        assert(RowIndex1 <= uRow && RowIndex2 <= uRow);

        T *pRow1Head = &ElemAt(RowIndex1, 1);
        T *pRow2Head = &ElemAt(RowIndex2, 1);
        T tmp;
        for (size_t i = 0; i < uCol; ++i)
        {
            tmp = pRow1Head[i];
            pRow1Head[i] = pRow2Head[i];
            pRow2Head[i] = tmp;
        }
    }

private:
    /**
        @brief 矩阵初等变换：倍乘行：将目标行乘以k倍。
        
        使用初等变换函数相比左乘变换方阵更加高效

        若定义了MATRIX_INDEX_START_AT_0,则行序号从0开始，
        否则从1开始。

        @param RowIndex	目标行序号
        @param k		系数，不可为0
    */
    void RowScaling(size_t RowIndex, const T &k)
    {
        assert(RowIndex <= uRow && k != T(0));

        T *pRowHead = &ElemAt(RowIndex, 1);
        for (size_t i = 0; i < uCol; ++i)
            pRowHead[i] *= k;
    }

private:
    /**
        @brief 矩阵初等变换：倍加行：将源行各数据乘以系数加到目标行对应数据。
        
        使用初等变换函数相比左乘变换方阵更加高效.
        
        若定义了MATRIX_INDEX_START_AT_0,则行序号从0开始，
        否则从1开始。
        
        @param SrcRowIndex	源行序号
        @param k			系数，可以为0
        @param TagRowIndex	目标行序号
    */
    void RowAddition(size_t SrcRowIndex, const T &k, size_t TrgRowIndex)
    {
        assert(SrcRowIndex <= uRow && TrgRowIndex <= uRow);

        T *pSrcRowHead = &ElemAt(SrcRowIndex, 1);
        T *pTrgRowHead = &ElemAt(TrgRowIndex, 1);

        for (size_t i = 0; i < uCol; ++i)
            pTrgRowHead[i] += k * pSrcRowHead[i];
    }

public:
    /**
        @brief 清除某一行的数据

        若定义了MATRIX_INDEX_START_AT_0,则行序号从0开始，
        否则从1开始。

        @param RowIndex	要清除的行序号
    */
    void ClearRow(size_t RowIndex)
    {
#ifdef MATRIX_INDEX_START_AT_0
        ++RowIndex;
#endif
        assert(RowIndex <= uRow);

        T *pRowHead = &ElemAt(RowIndex, 1);
        for (size_t i = 0; i < uCol; ++i)
            pRowHead[i] = T(0);
    }

public:
    /**
        @brief 清除某一列的数据

        若定义了MATRIX_INDEX_START_AT_0,则列序号从0开始，
        否则从1开始。

        @param ColIndex	要清除的列序号
    */
    void ClearColumn(size_t ColIndex)
    {
#ifdef MATRIX_INDEX_START_AT_0
        ++ColIndex;
#endif
        assert(ColIndex <= uCol);

        T *pColHead = &ElemAt(1, ColIndex);
        for (size_t i = 0; i < uRow; ++i)
            pColHead[i * uCol] = T(0);
    }

public:
    /**
        @brief 矩阵求幂

        @param n	幂阶数
        @return		矩阵的n次幂矩阵
    */
    Matrix<T> Power(size_t n) const
    {
        assert(uRow == uCol);

        Matrix<T> r(uRow, uCol);
        for (size_t i = 1; i <= uRow; ++i)
            r.ElemAt(i, i) = T(1);

        for (size_t i = 0; i < n; ++i)
            r = r * (*this);

        return r;
    }

public:
    /**
        @brief 矩阵转置
        @return	矩阵的转置矩阵
    */
    Matrix<T> Transpose() const
    {
        Matrix<T> r(this->uCol, this->uRow);
        for (size_t i = 1; i <= r.uRow; ++i)
            for (size_t j = 1; j <= r.uCol; ++j)
                r.ElemAt(i, j) = this->ElementAt(j, i);
        return r;
    }

public:
    /**
        @brief 余子式矩阵
        @param m	元素所在的行数
        @param n	元素所在的列数
        @return		若为方阵，返回第m行第n列元素的余子式矩阵；否则返回空矩阵
    */
    Matrix<T> MinorOf(size_t m, size_t n) const
    {
        assert(uRow == uCol);

        Matrix<T> r(this->uRow - 1, this->uCol - 1);
        size_t index = 0;
        for (size_t i = 1; i <= uRow; ++i)
            for (size_t j = 1; j <= uCol; ++j)
                if (i != m && j != n)
                    r.pData[index++] = this->ElementAt(i, j);
        return r;
    }

public:
    /**
        @brief 行约化矩阵
        @return		矩阵的行约化结果矩阵
    */
    Matrix<T> RowReduce() const
    {
        Matrix<T> r(*this);

        //高斯消元化为行阶梯矩阵
        for (size_t i = 1; i <= r.uRow; ++i)
        {
            //在第i列的第i行到第n行元素中寻找不为0的主元
            size_t NoneZeroIndex = i;
            while (NoneZeroIndex <= r.uRow && r.ElemAt(NoneZeroIndex, i) == T(0))
                ++NoneZeroIndex;
            if (NoneZeroIndex == r.uRow + 1) //该列全部为0
                continue;
            else if (NoneZeroIndex != i) //非0主元不是(i,i)
                r.RowInterchange(NoneZeroIndex, i);

            //元素(i,i)变为1
            r.RowScaling(i, T(1) / r.ElemAt(i, i));

            //从第i+1行开始各行第i个元素化为0
            for (size_t j = i + 1; j <= r.uRow; ++j)
                r.RowAddition(i, -r.ElemAt(j, i), j);
        }

        //从下往上化简形成行最简矩阵
        for (size_t i = r.uRow; i > 0; --i)
        {
            if (r.ElemAt(i, i) == T(0))
                continue;
            for (size_t j = 1; j < i; ++j)
                r.RowAddition(i, -r.ElemAt(j, i), j);
        }

        //让所有(i,i)非零行在最上面
        for (size_t i = 1; i <= r.uRow; ++i)
        {
            if (r.ElemAt(i, i) != T(0))
                continue;

            size_t j = i;
            for (; j <= r.uRow; ++j)
                if (r.ElemAt(j, i) != T(0))
                {
                    r.RowInterchange(i, j);
                    break;
                }

            if (j == r.uRow + 1)
                break;
        }

        return r;
    }

private:
    //行最简矩阵求秩
    size_t RankOfReducedMatrix() const
    {
        size_t i = 1;
        for (; i <= uRow; ++i)
        {
            const T *pRowHead = &ElementAt(i, 1);
            size_t j = i - 1;
            for (; j < uCol; ++j)
                if (pRowHead[j] != T(0))
                    break;
            if (j == uCol)
                break;
        }
        return i - 1;
    }

public:
    /**
        @brief 矩阵求秩
        @return	矩阵的秩
    */
    size_t Rank() const
    {
        Matrix<T> reducedMat = this->RowReduce();
        return reducedMat.RankOfReducedMatrix();
    }

public:
    /**
        @brief 判断当前矩阵是否可逆
        @return	若可逆，返回true；否则返回false
    */
    bool Invertible() const
    {
        if (this->uRow != this->uCol || this->Rank() != this->uRow)
            return false;
        return true;
    }

public:
    /**
        @brief 矩阵求逆
        @return 若矩阵可逆，返回逆矩阵；否则返回空矩阵
    */
    Matrix<T> Inverse() const
    {
        //判断是否为方阵
        assert(uRow == uCol);

        //生成单位矩阵
        Matrix<T> E(uRow, uRow);
        for (size_t i = 1; i <= E.uRow; ++i)
            E.ElemAt(i, i) = T(1);
        //将单位阵合并在矩阵右侧并行约化
        Matrix<T> ReducedCombinedMat = this->CombineWith(E, RIGHT).RowReduce();
        //判断矩阵是否满秩（可逆）
        assert(ReducedCombinedMat.RankOfReducedMatrix() == uRow);

        //将右侧部分分割并返回
        return ReducedCombinedMat.ColumnSplit(this->uCol + 1, RIGHT);
    }

public:
    /**
     *  @brief 矩阵输出为字符串
     *  @return 矩阵流输出的字符串
     */
    std::string toString()
    {
        std::stringstream s;
        s << *this;
        return s.str();
    }

public:
    /**
     * @brief 生成一个全0矩阵
     * 
     * @param size 矩阵的阶数
     * @return Matrix<T> size * size 大小的全0矩阵
     */
    static Matrix<T> Zeroes(size_t size)
    {
        return Matrix<T>(size, size, T(0));
    }

public:
    /**
     * @brief 生成一个单位矩阵
     * 
     * @param size 矩阵的阶数
     * @return Matrix<T> size * size 大小的单位矩阵
     */
    static Matrix<T> Identity(size_t size)
    {
        Matrix<T> &&mat = Zeroes(size);
        for (size_t i = 1; i <= size; ++i)
            mat.ElemAt(i, i) = T(1);
        return mat;
    }

public:
    /**
     * @brief 生成一个全1矩阵
     * 
     * @param size 矩阵的阶数
     * @return Matrix<T> size * size 大小的全1矩阵
     */
    static Matrix<T> Ones(size_t size)
    {
        return Matrix<T>(size, size, T(1));
    }

public:
    /**
     * @brief 生成一个随机元素矩阵
     * 
     * 矩阵的元素在0~1之间随机取值，该函数要求double类型可以转换到模板参数T类型
     * 
     * @param rows  行数
     * @param cols  列数
     * @return Matrix<T> rows * cols 大小的随机矩阵
     */
    static Matrix<T> Rand(size_t rows, size_t cols)
    {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<> dis(0.0, 1.0);

        std::vector<T> values;
        values.reserve(rows * cols);
        for (size_t i = 0; i < rows * cols; ++i)
            values.emplace_back(dis(gen));

        return Matrix<T>(rows, cols, values);
    }

public:
    /**
     * @brief 生成一个随机元素矩阵
     * 
     * 矩阵的元素在0~1之间随机取值，该函数要求double类型可以转换到模板参数T类型
     * 
     * @param size  阶数
     * @return Matrix<T> size * size 大小的随机方阵
     */
    static Matrix<T> Rand(size_t size)
    {
        return Matrix<T>::Rand(size, size);
    }
};

/**
    @brief 矩阵流输出运算符重载

    要求矩阵数据T类已重载流输出运算符
*/
#include <iomanip>
template <typename T>
std::ostream &operator<<(std::ostream &os, const Matrix<T> &mat)
{
    T *pRowHead = mat.pData;
    os << "[\n";
    for (size_t i = 1; i <= mat.uRow; ++i)
    {
        for (size_t j = 0; j < mat.uCol; ++j)
            os << std::setw(12) << std::setfill(' ') << std::setprecision(4) << pRowHead[j];
        os << '\n';
        pRowHead += mat.uCol;
    }
    os << "]\n";
    return os;
}

/**
    矩阵数乘（数在左）：
    返回数左乘矩阵的结果
*/
template <typename T>
inline Matrix<T> operator*(const T &c, const Matrix<T> &mat)
{
    return mat * c;
}

/**
 * @brief 行列式
 * 
 * @tparam T 行列式数据类型 
 */
template <typename T>
class Determinant
{
private:
    Matrix<T> *pMat;
    size_t size = 0;

public:
    //构造函数

    /**
     * @brief 行列式构造函数：无参
     * 
     */
    Determinant() : Determinant(0) {}

    /**
     * @brief 行列式构造函数：定义阶数
     * 
     * @param _size 行列式阶数
     */
    Determinant(size_t _size) : size(_size)
    {
        pMat = new Matrix<T>(size, size);
    }

    /**
     * @brief 行列式构造函数：定义阶数,数据
     * 
     * @param _size     行列式阶数
     * @param data      行列式数据
     * @param dataLen   行列式数据长度
     */
    Determinant(size_t _size, const T *data, size_t dataLen) : Determinant(_size)
    {
        size_t elemCount = _size * _size;
        memcpy_s(pMat->pData, elemCount * sizeof(T), data,
                 dataLen > elemCount ? elemCount * sizeof(T) : dataLen * sizeof(T));
    }

    /**
     * @brief 行列式构造函数: 由方阵构造
     * 
     * @param mat 方阵
     */
    Determinant(const Matrix<T> &mat)
    {
        assert(mat.uRow == mat.uCol);
        pMat = new Matrix<T>(mat);
        size = mat.uRow;
    }

    /**
     * @brief 行列式拷贝构造函数
     * 
     * @param det 
     */
    Determinant(const Determinant<T> &det) : Determinant(*det.pMat) {}

    /**
     * @brief 行列式移动拷贝函数
     * 
     * @param det 
     */
    Determinant(Determinant<T> &&det) noexcept
    {
        this->size = det.size;
        this->pMat = det.pMat;
        det.pMat = nullptr;
    }

    //析构函数
    virtual ~Determinant()
    {
        if (pMat)
            delete pMat;
    }

    /**
     * @brief 行列式赋值运算符函数
     * 
     * @param det 
     * @return Determinant<T>& 
     */
    Determinant<T> &operator=(const Determinant<T> &det)
    {
        if (&det == this)
            return *this;

        pMat = new Matrix<T>(*det.pMat);

        this->size = pMat->uRow;

        return *this;
    }

    /**
     * @brief 行列式移动赋值函数
     * 
     * @param det 
     * @return Determinant<T>& 
     */
    Determinant<T> &operator=(Determinant<T> &&det) noexcept
    {
        if (&det == this)
            return *this;
        Matrix<T> &mat = *det.pMat;
        pMat->uRow = mat.uRow;
        pMat->uCol = mat.uCol;
        pMat->uCapacity = mat.uCapacity;
        pMat->pData = mat.pData;
        mat.pData = nullptr;

        this->size = mat.uRow;

        return *this;
    }

    /**
     * @brief 行列式求值
     * 
     * @return T 行列式的值
     */
    T Value() const
    {
        if (this->size == 1)
            return this->pMat->pData[0];

        T sum = T(0), sign = T(1);

        for (size_t i = 0; i < size; ++i)
        {
            Matrix<T> &&minorMat = pMat->MinorOf(1, i + 1);
            Determinant minorDet(minorMat);
            sum += sign * minorDet.Value() * pMat->pData[i];
            sign = -sign;
        }

        return sum;
    }
};
