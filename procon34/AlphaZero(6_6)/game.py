import random
import math
import numpy as np

# フィールド
WIDTH = 6
HEIGHT = 6
GAME_COUNT = 30 # ターン数

WALL_POINT = 2
AREA_POINT = 10
CASTLE_POINT = 20

NUM_CRAFTSMEN = 1

class State:
    # 局面の初期化
    def __init__(self, craftsmen=None, enemy_craftsmen=None, walls=None, enemy_walls=None, areas=None, enemy_areas=None, game_count=None):
        # フィールドの初期化
        # 職人の位置
        self.craftsmen = craftsmen if craftsmen != None else np.zeros((WIDTH, HEIGHT), dtype=np.bool8)
        self.enemy_craftsmen = enemy_craftsmen if enemy_craftsmen != None else np.zeros((WIDTH, HEIGHT))
        # 壁の位置
        self.walls = walls if walls != None else np.zeros((WIDTH, HEIGHT), dtype=np.bool8)
        self.enemy_walls = enemy_walls if enemy_walls != None else np.zeros((WIDTH, HEIGHT))
        # 領域の位置
        self.areas = areas if areas != None else np.zeros((WIDTH, HEIGHT), dtype=np.bool8)
        self.enemy_areas = enemy_areas if enemy_areas != None else np.zeros((WIDTH, HEIGHT), dtype=np.bool8)
        # 現在のターン数
        self.game_count = game_count if game_count != None else 0
        # 方向
        self.directions = np.array([[0,1],[-1,0],[0,-1],[1,0],[-1,1],[-1,-1],[1,-1],[1,1]]) # 上、左、下、右、左上、左下、右下、右上
        # 行動
        self.actions = np.zeros(NUM_CRAFTSMEN*17, dtype=np.bool8) # 0~7: 移動、8~11: 建築、12~15: 解体、16: 滞在

        self.num_area, self.num_enemy_area = 0, 0
        self.num_walls, self.num_enemy_walls = 0, 0
        self.num_castles, self.num_enemy_castles = 0, 0
        
        # 各職人をランダムにフィールドに配置
        if craftsmen == None and enemy_craftsmen == None:
            board = np.zeros((WIDTH, HEIGHT), dtype=np.uint8)
            while True:
                x = np.random.choice(WIDTH, NUM_CRAFTSMEN*2)
                y = np.random.choice(HEIGHT, NUM_CRAFTSMEN*2)
                ok = True
                # ally
                for i in range(NUM_CRAFTSMEN):
                    if board[x[i],y[i]]:
                        ok = False
                        break
                    board[x[i],y[i]] = 1
                # enemy
                for i in range(NUM_CRAFTSMEN, NUM_CRAFTSMEN*2):
                    if board[x[i],y[i]]:
                        ok = False
                        break
                    board[x[i],y[i]] = 2
                if ok: break
                board = np.zeros((WIDTH, HEIGHT), dtype=np.uint8)
            self.craftsmen[board == 1] = 1
            self.enemy_craftsmen[board == 2] = 1
    
    # デュアルネットワークの入力の二次元配列
    def pieces_array(self):
        pass

    # 領域の計算
    def calc_areas(self) -> None:
        next_area = np.zeros((WIDTH, HEIGHT), dtype=np.bool8)
        next_enemy_area = np.zeros((WIDTH, HEIGHT), dtype=np.bool8)

        self.areas = next_area
        self.enemy_areas = next_enemy_area
        self.num_area, self.num_enemy_area = next_area.sum(), next_enemy_area.sum()
        self.num_castles, self.num_enemy_castles = 0, 0

    
    def get_areas(self):
        return self.areas, self.enemy_areas
    
    def _calc_score_diff(self) -> int:
        return (self.num_area - self.num_enemy_area) * AREA_POINT + (self.num_walls - self.num_enemy_walls) * WALL_POINT + (self.num_castles - self.num_enemy_castles) * CASTLE_POINT
    
    def is_lose(self) -> bool:
        return self._calc_score_diff() < 0
    
    def is_draw(self) -> bool:
        return self._calc_score_diff() == 0
    
    def is_done(self) -> bool:
        return self.game_count >= GAME_COUNT

    # actionsから次の状態へ遷移
    def next(self):
        assert self.is_legal_action()
        state = State(craftsmen=self.craftsmen, enemy_craftsmen=self.enemy_craftsmen, walls=self.walls, enemy_walls=self.enemy_walls, areas=self.areas, enemy_areas=self.enemy_areas, game_count=self.game_count)
    
    def is_legal_action(self) -> bool:
        pass
    
    def __str__(self) -> str:
        pass

    def build_wall(self, action):
        craftsmen = np.where(self.craftsmen == 1) # 職人がいる場所をタプルで返す


# ランダムに行動させる関数
# あとで、前職人がいた場所に行動できないのを実装する
def random_action(state: State):
    while True:
        action = np.random.choice(3) # ランダムに、移動、建築、解体、滞在を選択
        if action == 0: # 移動だったら
            direction = np.random.choice(7)
        if state.is_legal_action():
            break
    
    return action, direction


if __name__ == '__main__':
    # 状態の生成
    state = State()

    # ゲーム終了までのループ
    while True:
        # ゲーム終了時
        if state.is_done():
            break

        # 次の状態の取得
        state = state.next(random_action(state))

        # 文字列表示
        print(state)
        print()