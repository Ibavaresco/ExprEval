A simple expression parser and evaluator based on a version I wrote over 37 years ago in Z80 assembly.
It is a work in progress, but it supports variables ( starting from 'a', 'b', 'c'... ) and operator precedence.
It uses one stack for the values and other for the operators.
It is appropriated for the PIC microcontrollers because it doesn't use recursion.

<pre>
Sample output for DATA_TYPE == TYPE_SHORT:

        1|356<<2+3/2 =         2849 (4105 tcy)
((((1|356)<<2)+3)/2) =          715 (5685 tcy)
Error: "         ()"                (451 tcy)
                -(1) =           -1 (1204 tcy)
                (-1) =           -1 (1200 tcy)
           (((1+2))) =            3 (2382 tcy)
Error: "          ("                (367 tcy)
Error: "          )"                (259 tcy)
Error: "         1("                (717 tcy)
Error: "         (1"                (735 tcy)
Error: "         )1"                (259 tcy)
Error: "         1)"                (587 tcy)
                (-1) =           -1 (1200 tcy)
        ((1<<2)+3)*3 =           21 (3524 tcy)
          (1<<2+3)*3 =           96 (3150 tcy)
         (1+2)*(2+3) =           15 (3432 tcy)
                   1 =            1 (519 tcy)
                   A =           10 (423 tcy)
                  +1 =            1 (726 tcy)
                  -1 =           -1 (812 tcy)
                  ~0 =           -1 (868 tcy)
                 ~-1 =            0 (1161 tcy)
                  !1 =            0 (840 tcy)
                  !0 =            1 (840 tcy)
                 3*5 =           15 (1274 tcy)
                30/5 =            6 (1722 tcy)
                 5%3 =            2 (1573 tcy)
                 3+5 =            8 (1208 tcy)
                 3-5 =           -2 (1209 tcy)
                1<<3 =            8 (1320 tcy)
                8>>3 =            1 (1306 tcy)
           65535&255 =          255 (2084 tcy)
           65535^255 =         -256 (2085 tcy)
           65280|255 =           -1 (2114 tcy)
                1<2  =            1 (1358 tcy)
                 2<1 =            0 (1315 tcy)
                1<=2 =            1 (1262 tcy)
                 1=2 =            0 (1254 tcy)
                 1=1 =            1 (1254 tcy)
                1==1 =            1 (1254 tcy)
                1!=1 =            0 (1223 tcy)
                1!=2 =            1 (1223 tcy)
                1>=2 =            0 (1264 tcy)
                2>=1 =            1 (1264 tcy)
                 2>1 =            1 (1290 tcy)
                 2>2 =            0 (1290 tcy)
                1!=1 =            0 (1223 tcy)
                1!=2 =            1 (1223 tcy)
                1<>1 =            0 (1279 tcy)
                1<>2 =            1 (1279 tcy)
            1<2&&4>3 =            1 (2800 tcy)
            1>2&&4>3 =            0 (2774 tcy)
            1<2&&4<3 =            0 (2818 tcy)
            1>2&&4<3 =            0 (2793 tcy)
            1<2||4>3 =            1 (2825 tcy)
            1>2||4>3 =            1 (2800 tcy)
            1<2||4<3 =            1 (2853 tcy)
            1>2||4<3 =            0 (2827 tcy)
           100+100/3 =          133 (2866 tcy)


Sample output for DATA_TYPE == TYPE_DOUBLE:

        1|356<<2+3/2 =  2849.000000 (12801 tcy)
((((1|356)<<2)+3)/2) =   715.500000 (14508 tcy)
Error: "         ()"                (453 tcy)
                -(1) =    -1.000000 (2135 tcy)
                (-1) =    -1.000000 (2131 tcy)
           (((1+2))) =     3.000000 (4351 tcy)
Error: "          ("                (369 tcy)
Error: "          )"                (260 tcy)
Error: "         1("                (1598 tcy)
Error: "         (1"                (1617 tcy)
Error: "         )1"                (260 tcy)
Error: "         1)"                (1467 tcy)
                (-1) =    -1.000000 (2131 tcy)
        ((1<<2)+3)*3 =    21.000000 (8697 tcy)
          (1<<2+3)*3 =    96.000000 (8274 tcy)
         (1+2)*(2+3) =    15.000000 (7977 tcy)
                   1 =     1.000000 (1415 tcy)
                   A =    10.000000 (467 tcy)
                  +1 =     1.000000 (1623 tcy)
                  -1 =    -1.000000 (1741 tcy)
                  ~0 =    -1.000000 (1865 tcy)
                 ~-1 =     0.000000 (2391 tcy)
                  !1 =     0.000000 (1810 tcy)
                  !0 =     1.000000 (1802 tcy)
                 3*5 =    15.000000 (3651 tcy)
                30/5 =     6.000000 (4982 tcy)
                 5%3 =     2.000000 (4950 tcy)
                 3+5 =     8.000000 (3172 tcy)
                 3-5 =    -2.000000 (3192 tcy)
                1<<3 =     8.000000 (3872 tcy)
                8>>3 =     1.000000 (3824 tcy)
           65535&255 =   255.000000 (10813 tcy)
           65535^255 = 65280.000000 (10726 tcy)
           65280|255 = 65535.000000 (10297 tcy)
                1<2  =     1.000000 (3513 tcy)
                 2<1 =     0.000000 (3210 tcy)
                1<=2 =     1.000000 (3417 tcy)
                 1=2 =     0.000000 (3119 tcy)
                 1=1 =     1.000000 (3399 tcy)
                1==1 =     1.000000 (3399 tcy)
                1!=1 =     0.000000 (3108 tcy)
                1!=2 =     1.000000 (3348 tcy)
                1>=2 =     0.000000 (3159 tcy)
                2>=1 =     1.000000 (3419 tcy)
                 2>1 =     1.000000 (3445 tcy)
                 2>2 =     0.000000 (3178 tcy)
                1!=1 =     0.000000 (3108 tcy)
                1!=2 =     1.000000 (3348 tcy)
                1<>1 =     0.000000 (3164 tcy)
                1<>2 =     1.000000 (3404 tcy)
            1<2&&4>3 =     1.000000 (7439 tcy)
            1>2&&4>3 =     0.000000 (6893 tcy)
            1<2&&4<3 =     0.000000 (6935 tcy)
            1>2&&4<3 =     0.000000 (6650 tcy)
            1<2||4>3 =     1.000000 (7462 tcy)
            1>2||4>3 =     1.000000 (7177 tcy)
            1<2||4<3 =     1.000000 (7232 tcy)
            1>2||4<3 =     0.000000 (6686 tcy)
           100+100/3 =   133.333328 (9586 tcy)
</pre>
