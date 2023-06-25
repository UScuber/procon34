import random
import math
import numpy as np

# フィールド
WIDTH = 6
HEIGHT = 6

class State:
    # 局面の初期化
    def __init__(self, craftsmen=None, enemy_craftsmen=None, wall=None, enemy_wall=None):
        # フィールドの初期化
        self.craftsmen = np.zeros((WIDTH, HEIGHT))
        self.enemy_craftsmen = np.zeros((WIDTH, HEIGHT))
        self.wall = np.zeros((WIDTH, HEIGHT))
        self.enemy_wall = np.zeros((WIDTH, HEIGHT))
        self.area = np.zeros((WIDTH, HEIGHT))
        self.enemy_area = np.zeros((WIDTH, HEIGHT))
        
        # 各職人をランダムにフィールドに配置
        while True:
            x = np.random.choice(WIDTH, 2)
            y = np.random.choice(HEIGHT, 2)
            if x[0] != x[1] or y[0] != y[1]:
                break
        self.craftsmen[x[0], y[0]] = 1
        self.enemy_craftsmen[x[1], y[1]] = 1
    
    # デュアルネットワークの入力の二次元配列
    def pieces_array(self):

    # 領域の計算
    def colcu_areas(self):
        # 領域の計算
    
    # 領域の大きさを計算
    def get_areas(self):
        # 領域の大きさを計算
        return areas, enemy_areas # 関数名変えても構わない
    # 負けかどうか
    def is_lose(self):
    
    # 引き分けかどうか
    def is_draw(self):
    
    # ゲーム終了かどうか
    def is_done(self):
    
    # 次の状態の取得
    def next(self, action):
    
    # 合法手かどうか
    def is_leagal_action(self):
    
    # 文字列表示