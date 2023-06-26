import random
import math
import numpy as np

# フィールド
WIDTH = 6
HEIGHT = 6
GAME_COUNT = 30 # ターン数

class State:
    # 局面の初期化
    def __init__(self, craftsmen=None, enemy_craftsmen=None, walls=None, enemy_walls=None, areas=None, enemy_areas=None, game_count=None):
        # フィールドの初期化
        # 職人の位置
        self.craftsmen = craftsmen if craftsmen != None else np.zeros((WIDTH, HEIGHT))
        self.enemy_craftsmen = enemy_craftsmen if enemy_craftsmen != None else np.zeros((WIDTH, HEIGHT))
        # 壁の位置
        self.walls = walls if walls != None else np.zeros((WIDTH, HEIGHT))
        self.enemy_walls = enemy_walls if enemy_walls != None else np.zeros((WIDTH, HEIGHT))
        # 領域の位置
        self.areas = areas if areas != None else np.zeros((WIDTH, HEIGHT))
        self.enemy_areas = enemy_areas if enemy_areas != None else np.zeros((WIDTH, HEIGHT))
        # 現在のターン数
        self.game_count = game_count if game_count != None else 0
        # 方向
        self.directions = np.array([[0,1],[-1,0],[0,-1],[1,0],[-1,1],[-1,-1],[1,-1],[1,1]]) # 上、左、下、右、左上、左下、右下、右上
        # 行動
        self.actions = np.zeros(17, dtype=np.int8) # 0~7: 移動、8~11: 建築、12~15: 解体、16: 滞在
        
        # 各職人をランダムにフィールドに配置
        if craftsmen == None and enemy_craftsmen == None:
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
        if self.game_count >= GAME_COUNT:
            return True
        return False

    # 次の状態の取得
    def next(self, action):
    
    # 合法手かどうか
    def is_leagal_action(self):
    
    # 文字列表示

    # 建築
    def build_wall(self, action):
        craftsmen = np.where(self.craftsmen == 1) # 職人がいる場所をタプルで返す



