## ランダムエージェント


### 概要

職人を合法手となるもののうち、ランダムに行動を決める

味方の行動をランダムに決定する


### 入力形式

- 構造(S): { 0:none, 1:pond, 2:castle }

- 城壁(W): { 0:none, 1:ally, 2:enemy }

- 陣地(R): { 0:none, 1:ally, 2:enemy, 3:both }

- 職人(A): { ally:positive integer, enemy:negative integer }

```
H W N turn side

S(0,0) S(0,1) ... S(0,W-1)
...
S(H-1,0), S(H-1,0) ... S(H-1,W-1)

W(0,0) W(0,1) ... W(0,W-1)
...
W(H-1,0), W(H-1,0) ... W(H-1,W-1)

R(0,0) R(0,1) ... R(0,W-1)
...
R(H-1,0), R(H-1,0) ... R(H-1,W-1)

A(0,0) A(0,1) ... A(0,W-1)
...
A(H-1,0), A(H-1,0) ... A(H-1,W-1)
```


### 出力形式

- 行動: { 0:none, 1:move, 2:build, 3:break }

- 向き: { 0:none, 1:LU, 2:U, 3:RU, 4:R, 5:RD, 6:D, 7:LD, 8:L }

```
<職人0の行動> <職人0の向き>
...
<職人N-1の行動> <職人N-1の向き>
```
