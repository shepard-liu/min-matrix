# min-matrix
Minimal matrix implementation in C++.

:hugs:[Pull Request](https://github.com/shepard-liu/min-matrix/pulls/):hugs:  :sunglasses:[Post Issues](https://github.com/shepard-liu/min-matrix/issues/):sunglasses:

## Brief

😄```Eigen``` compiles too slow?😄	😅Just want something simple and convenient?😅
**Take 5 minutes with this repo!**
Basic implementations of Matrix object facilities and algebra algorithms are included. APIs are designed elegantly and fully equipped with parameter annotations(**in Chinese**).

* Construct

```Matrix<T>``` is a well-designed matrix class that provides convenient constructors and generation functions like ```Matrix<T>::Zeroes()```, ```Matrix<T>::Identity()```, ```Matrix<T>::Ones()```, ```Matrix<T>::Rand()```. Constructors like ```Matrix<T>::Matrix(const std::string &expr)``` and ```Matrix(std::initializer_list<std::initializer_list<T>> iList)``` parse MATLAB-style literals after performing semantic checks.

* Matrix Operations

'+', '-' and '*' are overloaded to support add, substract and matrix multiply respectively.
Member functions such as ```Matrix<T>::Power()```, ```Matrix<T>::Transpose()``` and ```Matrix<T>::Inverse()``` perform basic algebra algorithms.

* Easy-to-use Manipulation Methods

To add a row or column, you may invoke ```Matrix<T>::AddRow()``` or ```Matrix<T>::AddColumn```. 
```Matrix<T>::CombineWith()``` handles concatenation of two matrices with ease.

* Customize Your experience

Prefer using 1-based index with MATLAB 😀? Want to implement your business logic after a hard time with mathematical formulas:upside_down_face:? min-matrix comes to your help. **By default, row or column indices begin with 1**. If you wanna switch to 0-based which is loved by programmers, add ```#define MATRIX_INDEX_START_AT_0``` before you the matrix header file.
    
* More

Discover by yourself down below:grin:!

## Tutorials

1. Constructing a matrix object

    * Basic constructors
    
    Construct a Matrix object by defining the size.
    
    ```C++
    // Default constructor
    Matrix<double> mat0();
    // Constuct a matrix of 5 rows and 3 columns
    Matrix<double> mat1(5,3);
    ```
    
    * Data constructors
    
    Construct a Matrix object by defining the size and supplying data linearly.
    
    ```C++
    // Constuct a square matrix of 2 rows with data
    Matrix<double> mat2_1(2, 2, {1.3e2, .5, -2.87, 3.14});
    // InitializerList<T> will be implicitly converted to a vector
    /*
    mat2_1
        1.3e2   0.5
        -2.87   3.14
    */

    // You may leave some elements unassigned, which will be T()
    // In this case, "double() == 0"
    Matrixd mat2_2(2, 2, {1.3e2, .5}); 
    // typedef Matrix<double> Matrixd;
    /*
    mat2_2
        1.3e2   0.5
        0       0
    */
    
    // The data can also be served with arrays
    const double data[] = {2.2, 1.4, .5, -0.2};
    // The 4th argument should be the length of data array
    Matrixd mat2_3(2, 3, data, 4);
    /*
    mat2_3
        2.2   1.4   .5
        -0.2  0     0
    */

    // A single value can be used to construct a Matrix object
    Matrixd mat3(3, 1, 1.5);
    /*
    mat3
        1.5
        1.5
        1.5
    */
    ```
    
    * Special Constructors
    
    You don't have to specify the row or column size but to provide structured data to construct an object.
    
    ```C++
    // MATLAB-style literals are well supported
    Matrixd mat4("[2.3 -3.14, 21;  0.2,2.9 0.02; 0.77, -0.5,8]");
    // Delimeter can be either ' ' or ',' or both
    /*
    mat4
        2.3     -3.14   21
        0.2     2.9     0.02
        0.77    -0.5    8
    */
    ```
    
    If a semantic issue is detected, the constructor function prints out a error message on the console. 

    ```C++
    Matrixd("[2.3 -3.14, 21; xxx 0.2,2.9 0.02; 0.77, -0.5,8]");
    /*
        Error parsing matrix expression (unable to parse row element):

        [2.3 -3.14, 21; xxx 0.2,2.9 0.02; 0.77, -0.5,8]
                        ^
        Assertion failed!
    */
    ```
    
    ```C++
    // Nested InitializerList can be used
    Matrixd mat5({ {2, 3}, {9, 5} });
    /*
    mat5
        2  3
        9  5
    */
    ```
    In both constructors above, the column size of the matrix is infered from the **first row** data provided. Be sure that all rows have equivalent amount of elements.

2. Generate a special or frequenly-used matrix

    ```C++
    // Special matrix can be generated with static member functions
    Matrixd mat8 = Matrixd::Identity(3);
    /*
    mat8
        1	0	0
        0	1	0
        0	0	1
    */

    Matrixd mat9 = Matrixd::Zeroes(4);
    /*
    mat9
        0	0	0	0
        0	0	0	0
        0	0	0	0
        0	0	0	0
    */

    Matrixd mat10 = Matrixd::Ones(2);
    /*
    mat10
        1	1
        1	1
    */

    Matrixd mat11 = Matrixd::Rand(3, 2);
    // mat11 will be a matrix of 3 rows and 2 columns with
    // random elements uniformly distributed between 0.0~1.0
    /*
        mat11
        I have no idea how it'll be like.
    */
    ```
    
3. Modify Elements

     In most libraries matrix index(row or column) begin with 0, while in MATLAB it is 1. So we leave it up to users that if macro MATRIX_INDEX_START_AT_0 is defined before you include "Matrix.h", the beginning index will be. This setting up will only affect the implementation of ```Matrix<T>::operator()```. Other functions like ```ElemAt()```, ```ElemAt0()``` acts as the document says anyway.

    ```C++
    Matrixd mat12(2, 3, {0, 1, 2, 3, 4, 5});
    /*
    mat12
        0	1	2
        3	4	5
    */
    mat12(1, 2);
    //if defined MATRIX_INDEX_START_AT_0 		: 5
    //if not defined MATRIX_INDEX_START_AT_0	: 1

    mat12.ElemAt0(1, 2); // 5
    mat12.ElemAt(1, 2);  // 1
    ```
    
    In order to traverse the matrix elements and modify them, you surely can write a for statement yourself. However, you may also pass a lambda function as a parameter to ```Matrix<T>::ForEach()```.

    ```C++
    // Traverse
    mat12.ForEach([](double &elem, size_t index)
    {
        if (elem < 5)
            elem = 0;
        if (index >= 2)
            return false;
        return true;
    });
    /*
	mat12
		0	0	0
		3	4	5
	*/
    ```

4. Basic matrix operations

    ```C++
    Matrixd mat13_1(2, 3, {1, 3, 4, 2, 5, 9});
    Matrixd mat13_2(2, 3, {1, -1, 3, -2, 2, -4});
    Matrixd mat13_3(3, 3, {1, 1, 2, 2, 5, 2, 0, 4, 2});

    // Add
    Matrixd mat13_4 = mat13_1 + mat13_2;
    /*
    mat13_4
        2    2    7
        0    7    5
    */

    // Substract
    Matrixd mat13_5_1 = mat13_1 - mat13_2;
    /*
    mat13_5_1
        0   4   1
        4   3   13
    */

    // Negative
    Matrixd mat13_5_2 = -mat13_1;
    /*
    mat13_5_2
        -1   -3   -4
        -2   -5   -9
    */

    // Multiply
    Matrixd mat13_6 = mat13_2 * mat13_3;
    /*
    mat13_6
        -1   8    6
        2    -8   -8
    */

    // Power
    Matrixd mat13_7 = mat13_3.Power(3); //mat13_3 * mat13_3 * mat13_3
    /*
    mat13_7
        31    105    50
        82    259    130
        64    196    96
    */

    // Transpose
    Matrixd mat13_8 = mat13_1.Transpose();
    /*
    mat13_8
        1    2
        3    5
        4    9
    */

    // Row Reduce
    Matrixd mat13_9 = mat13_3.RowReduce();
    /*
    mat13_9
        1   0   0
        0   1   0
        0   0   1
    */

    // Rank
    size_t rank = mat13_3.Rank();
    // rank == 3

    // Inverse
    if(mat13_3.Invertible()){
        Matrixd mat13_10 = mat13_3.Inverse();
    }
    /*
    mat13_10
        0.142857     0.428571     -0.571429
        -0.285714    0.142857     0.142857
        0.571429     -0.285714    0.214286
    */
    ```
    
5. Matrix concatenation

    ```C++
    Matrixd mat14({{2, 3, 4}, {3, 4, 5}, {5, 6, 7}});
    Matrixd mat15 = mat14.CombineWith(Matrixd::Identity(3), Matrixd::RIGHT);
    /*
    mat15
        2   3   4   1   0   0
        3   4   5   0   1   0
        5   6   7   0   0   1
    */
    ```

6. Modify an entire row or column
    
    ```C++
    Matrixd mat16({{2, 3, 4}, {3, 4, 5}, {5, 6, 7}});
    mat16.ClearColumn(1);
    /*
    mat16
        2   0   4
        3   0   5
        5   0   7
    */

    mat16.AddColumn({2, 4, 5});
    /*
    mat16
        2   0   4   2
        3   0   5   4
        5   0   7   5
    */

    mat16.InsertRow(0, {-1, -1, -1, -1});
    /*
    mat16
        -1  -1  -1  -1
        2   0   4   2
        3   0   5   4
        5   0   7   5
    */
    ```

    You can also extract a block of elements from the matrix with ```Matrix<T>::Block()```. The block area is copied from the original matrix.

    ```C++
    // 2nd ~ 3rd row and 2nd ~ 4th column will be extracted
    Matrixd blockMat = mat16.Block(1, 1, 2, 3);
    /*
    blockMat
        0    4    2
        0    5    4
    */
    ```

7. Determinant evaluate

    ```C++
    Matrixd mat17({{2, 1, 4}, {0, 2, 5}, {9, 6, 7}});
    double detValue = Determinant<double>(mat17).Value();
    // detValue == -59
    ```
