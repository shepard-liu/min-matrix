# min-matrix
Minimal matrix implementation in C++

```
    ////////////////////////////////
    //         Contructors        //
    ////////////////////////////////

    // Default constructor
    Matrix<double> mat0();
    // Constuct a matrix of 5 rows and 3 columns
    Matrix<double> mat1(5,3);
    // Constuct a square matrix of 2 rows with data
    Matrix<double> mat2_1(2, 2, {1.3e2, .5, -2.87, 3.14});
	// InitializerList<T> will be implicitly converted to a vector
    /*
    mat2_1
        1.3e2   0.5
        -2.87   3.14
    */
    VX(mat2_1);

    // You may leave some elements unassigned, which will be T()
    // In this case, "double() == 0"
    Matrixd mat2_2(2, 2, {1.3e2, .5}); 
    // typedef Matrix<double> Matrixd;
    /*
    mat2_2
        1.3e2   0.5
        0       0
    */
    VX(mat2_2);

    // The data can also be served with arrays
    const double data[] = {2.2, 1.4, .5, -0.2};
    // The 4th argument should be the length of data array
    Matrixd mat2_3(2, 3, data, 4);
    /*
    mat2_3
        2.2   1.4   .5
        -0.2  0     0
    */
    VX(mat2_3);

    // A single value can be used to construct a Matrix object
    Matrixd mat3(3, 1, 1.5);
    /*
    mat3
        1.5
        1.5
        1.5
    */
    VX(mat3);

    // MATLAB-style literals are well supported
    Matrixd mat4("[2.3 -3.14, 21;  0.2,2.9 0.02; 0.77, -0.5,8]");
    // Delimeter can be either ' ' or ',' or both
    /*
    mat4
        2.3     -3.14   21
        0.2     2.9     0.02
        0.77    -0.5    8
    */
    VX(mat4);

    // Nested InitializerList can be used
    Matrixd mat5({ {2, 3}, {9, 5} });
    /*
    mat5
        2  3
        9  5
    */
    VX(mat5);

    // Matrix<> provides a move constructor and move assignment operator
	// a rvalue can be efficiently reused
	Matrixd mat6(mat4 * mat4);
	Matrixd mat7 = mat4 * mat4;

    ////////////////////////////////
    //         Generators         //
    ////////////////////////////////

	// Special matrix can be generated with static member functions
	Matrixd mat8 = Matrixd::Identity(3);
    /*
    mat8
        1	0	0
		0	1	0
		0	0	1
    */
    VX(mat8);

    Matrixd mat9 = Matrixd::Zeroes(4);
    /*
    mat9
        0	0	0	0
		0	0	0	0
		0	0	0	0
		0	0	0	0
    */
    VX(mat9);

    Matrixd mat10 = Matrixd::Ones(2);
    /*
    mat10
        1	1
		1	1
    */
    VX(mat10);

    Matrixd mat11 = Matrixd::Rand(3, 2);
	// mat11 will be a matrix of 3 rows and 2 columns with
	// random elements uniformly distributed between 0.0~1.0
    VX(mat11);

    ////////////////////////////////
    //       Modify Elements      //
    ////////////////////////////////

	// In most libraries matrix index(row or column) begin with 0,
	// while in MATLAB it is 1. So we leave it up to users that if
	// macro MATRIX_INDEX_START_AT_0 is defined before you include
	// "Matrix.h", the beginning index will be. This setting up will
	// only affect the implementation of 
	// "T& Matrix<T>::operator()(size_t row, size_t col)".
	// Other functions like ElemAt(), ElemAt0() acts as the document
	// says anyway.

	Matrixd mat12(2, 3, {0, 1, 2, 3, 4, 5});
	/*
	mat12
		0	1	2
		3	4	5
	*/
    mat12(1, 2);
    VX(mat12(1, 2));
    //if defined MATRIX_INDEX_START_AT_0 		: 5
	//if not defined MATRIX_INDEX_START_AT_0	: 1

	mat12.ElemAt0(1, 2); // 5
    mat12.ElemAt(1, 2);  // 1
    VX(mat12.ElemAt0(1, 2));
    VX(mat12.ElemAt(1, 2));

    ////////////////////////////////
    //  Basic Matrix Operations   //
    ////////////////////////////////

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
    VX(mat13_4);

    // Substract
    Matrixd mat13_5_1 = mat13_1 - mat13_2;
    /*
    mat13_5_1
        0   4   1
        4   3   13
    */
    VX(mat13_5_1);

    // Negative
    Matrixd mat13_5_2 = -mat13_1;
    /*
    mat13_5_2
        -1   -3   -4
        -2   -5   -9
    */
    VX(mat13_5_2);

    // Multiply
    Matrixd mat13_6 = mat13_2 * mat13_3;
    /*
    mat13_6
        -1   8    6
        2    -8   -8
    */
    VX(mat13_6);

    // Power
    Matrixd mat13_7 = mat13_3.Power(3); //mat13_3 * mat13_3 * mat13_3
    /*
    mat13_7
        31    105    50
        82    259    130
        64    196    96
    */
    VX(mat13_7);

    // Transpose
    Matrixd mat13_8 = mat13_1.Transpose();
    /*
    mat13_8
        1    2
        3    5
        4    9
    */
    VX(mat13_8);

    // Row Reduce
    Matrixd mat13_9 = mat13_3.RowReduce();
    /*
    mat13_9
        1   0   0
        0   1   0
        0   0   1
    */
    VX(mat13_9);

    // Rank
    size_t rank = mat13_3.Rank();
    // 3
    VX(rank);

    // Inverse
    if(mat13_3.Invertible()){
        Matrixd mat13_10 = mat13_3.Inverse();
        VX(mat13_10);
    }
    /*
    mat13_10
        0.142857     0.428571     -0.571429
        -0.285714    0.142857     0.142857
        0.571429     -0.285714    0.214286
    */

    ////////////////////////////////
    //       Concatenation        //
    ////////////////////////////////

    Matrixd mat14({{2, 3, 4}, {3, 4, 5}, {5, 6, 7}});
    Matrixd mat15 = mat14.CombineWith(Matrixd::Identity(3), Matrixd::RIGHT);
    /*
    mat15
        2   3   4   1   0   0
        3   4   5   0   1   0
        5   6   7   0   0   1
    */
    VX(mat15);

    ////////////////////////////////
    //   Modify Rows and Columns  //
    ////////////////////////////////

    Matrixd mat16({{2, 3, 4}, {3, 4, 5}, {5, 6, 7}});
    mat16.ClearColumn(1);
    /*
    mat16
        2   0   4
        3   0   5
        5   0   7
    */
    VX(mat16);

    mat16.AddColumn({2, 4, 5});
    /*
    mat16
        2   0   4   2
        3   0   5   4
        5   0   7   5
    */
    VX(mat16);

    mat16.InsertRow(0, {-1, -1, -1, -1});
    /*
    mat16
        -1  -1  -1  -1
        2   0   4   2
        3   0   5   4
        5   0   7   5
    */
    VX(mat16);

    ////////////////////////////////
    //         Determinent        // 
    ////////////////////////////////

    Matrixd mat17({{2, 1, 4}, {0, 2, 5}, {9, 6, 7}});
    double detValue = Determinant<double>(mat17).Value();
    // -59
    VX(detValue);
```