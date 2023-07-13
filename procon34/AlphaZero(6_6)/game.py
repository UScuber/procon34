import random
import math
import numpy as np
from collections import deque

# フィールド
WIDTH = 6
HEIGHT = 6
GAME_COUNT = 30 # ターン数

WALL_POINT = 2
AREA_POINT = 10

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
        # self.actions = np.zeros(NUM_CRAFTSMEN*17, dtype=np.bool8) # 0~7: 移動、8~11: 建築、12~15: 解体、16: 滞在

        self.num_area, self.num_enemy_area = 0, 0
        self.num_walls, self.num_enemy_walls = 0, 0
        
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
        AREA = 1
        NEUTRAL = 2

        def calc_region(wall_func) -> np.ndarray:
            used = np.zeros((WIDTH, HEIGHT), dtype=np.uint8)
            que = deque()
            for i in range(HEIGHT):
                if not wall_func(0, i):
                    que.append((0, i))
                    used[0, i] = NEUTRAL
                if not wall_func(WIDTH-1, i):
                    que.append((WIDTH-1, i))
                    used[WIDTH-1, i] = NEUTRAL
                
            for j in range(WIDTH):
                if not wall_func(j, 0):
                    que.append((j, 0))
                    used[j, 0] = NEUTRAL
                if not wall_func(j, HEIGHT-1):
                    que.append((j, HEIGHT-1))
                    used[j, HEIGHT-1] = NEUTRAL
            while que:
                posx,posy = que.popleft()
                for i in range(4):
                    x = posx + self.directions[i][0]
                    y = posy + self.directions[i][1]
                    if x < 0 or y < 0 or x >= WIDTH or y >= HEIGHT: continue
                    if used[x, y] == 0 and not wall_func(x,y):
                        used[x, y] = NEUTRAL
                        que.append((x, y))
            
            for i in range(1, HEIGHT-1):
                for j in range(1, WIDTH-1):
                    if used[i,j] == 0 and not wall_func(j,i):
                        used[i,j] = AREA
            return used
        
        ally_region = calc_region(self.is_ally_wall)
        enemy_region = calc_region(self.is_enemy_wall)
        
        for i in range(HEIGHT):
            for j in range(WIDTH):
                if ally_region[j,i] == AREA and enemy_region[j,i] == AREA:
                    self.areas[j,i] = 1
                    self.enemy_areas[j,i] = 1
                elif ally_region[j,i] == AREA:
                    self.areas[j,i] = 1
                    self.enemy_areas[j,i] = 0
                elif enemy_region[j,i] == AREA:
                    self.areas[j,i] = 0
                    self.enemy_areas[j,i] = 1
                if self.is_ally_wall(j,i):
                    self.areas[j,i] = 0
                if self.is_enemy_wall(j,i):
                    self.enemy_areas[j,i] = 0

        self.num_area, self.num_enemy_area = self.areas.sum(), self.enemy_areas.sum()

    
    def get_areas(self):
        return self.areas, self.enemy_areas
    
    def _calc_score_diff(self) -> int:
        return (self.num_area - self.num_enemy_area) * AREA_POINT + (self.num_walls - self.num_enemy_walls) * WALL_POINT
    
    def is_lose(self) -> bool:
        return self._calc_score_diff() < 0
    
    def is_draw(self) -> bool:
        return self._calc_score_diff() == 0
    
    def is_done(self) -> bool:
        return self.game_count >= GAME_COUNT
    
    # 敵の壁か味方の壁か判定する
    def is_ally_wall(self, x, y) -> bool:
        return self.walls[x][y]
    def is_enemy_wall(self, x, y) -> bool:
        return self.enemy_walls[x][y]

    # actionsから次の状態へ遷移
    def next(self, actions):
        # 破壊建築移動の順番で作る(後で)
        assert self.is_legal_action(actions)
        craftsmen = np.where(self.craftsmen == 1) # 職人がいる場所をタプルで返す
        # 修正する

        for i in range(NUM_CRAFTSMEN):
            for j in range(17):
                # 移動だったら
                if 0<=j and j<=7 and actions[j*(i+1)]:
                    direction = self.directions[j]
                    x = craftsmen[0][i]
                    y = craftsmen[1][i]
                    next_place_x = x + direction[0]
                    next_place_y = y + direction[1]
                    # 職人を移動させる
                    self.craftsmen[x][y] = 0
                    self.craftsmen[next_place_x][next_place_y] = 1
                # 建築だったら
                elif 8<=j and j<=11 and actions[j*(i+1)]:
                    direction = self.directions[j]
                    x = craftsmen[0][i]
                    y = craftsmen[1][i]
                    build_place_x = x + direction[0]
                    build_place_y = y + direction[1]
                    # 壁を建築する
                    self.walls[build_place_x][build_place_y] = 1
                # 解体だったら
                elif 12<=j and j<=15 and actions[j*(i+1)]:
                    direction = self.directions[j]
                    x = craftsmen[0][i]
                    y = craftsmen[1][i]
                    dismantle_place_x = x + direction[0]
                    dismantle_place_y = y + direction[1]
                    # 壁を解体する
                    if self.is_ally_wall(dismantle_place_x, dismantle_place_y): # 味方なら
                        self.walls[dismantle_place_x][dismantle_place_y] = 0
                    elif self.is_enemy_wall(dismantle_place_x, dismantle_place_y): # 敵なら
                        self.enemy_walls[dismantle_place_x][dismantle_place_y] = 0
                    else:
                        pass
                #滞在なら
                elif j == 16 and actions[j*(i+1)]:
                    pass
        
        return State(craftsmen=self.enemy_craftsmen, enemy_craftsmen=self.craftsmen, walls=self.enemy_walls, enemy_walls=self.walls, areas=self.enemy_areas, enemy_areas=self.areas, game_count=(self.game_count+1))
    # 合法手かどうか確認する
    def is_legal_action(self, actions) -> bool:
        craftsmen = np.where(self.craftsmen == 1) # 職人がいる場所をタプルで返す
        enemy_craftsmen = np.where(self.enemy_craftsmen == 1)

        for i in range(NUM_CRAFTSMEN):
            for j in range(17):
                if actions[j*(i+1)]:
                    direction = self.directions[j]
                    x = craftsmen[0][i]
                    y = craftsmen[1][i]
                    next_place_x = x + direction[0]
                    next_place_y = y + direction[1]
                    # 移動
                    if 0 <= j and j <= 7:
                        # フィールド外なら
                        if next_place_x < 0 or WIDTH <= next_place_x:
                            return False
                        if next_place_y < 0 or HEIGHT <= next_place_y:
                            return False
                        # 相手の職人がいたら（あとで自分の職人とも重なっていないかの判定を加える）
                        if next_place_x == enemy_craftsmen[0][i] and next_place_y == enemy_craftsmen[1][i]:
                            return False
                        # 相手の壁があったら
                        if self.enemy_walls[next_place_x] == 1:
                            return False
                    # 建築
                    if 8 <= j and j <= 11:
                        # フィールド外なら
                        if next_place_x < 0 or WIDTH <= next_place_x:
                            return False
                        if next_place_y < 0 or HEIGHT <= next_place_y:
                            return False
                        # 相手の職人がいたら
                        if next_place_x == enemy_craftsmen[0][i] and next_place_y == enemy_craftsmen[1][i]:
                            return False
                        # 壁があったら
                        if self.enemy_walls[next_place_x][next_place_y] == 1 and self.walls[next_place_x][next_place_y]:
                            return False
                    # 解体
                    if 12 <= j and j <= 15:
                        # フィールド外なら
                        if next_place_x < 0 or WIDTH <= next_place_x:
                            return False
                        if next_place_y < 0 or HEIGHT <= next_place_y:
                            return False
                        # 指定先に壁がなかったら
                        if self.walls[next_place_x][next_place_y] or self.enemy_walls[next_place_x][next_place_y]:
                            return False
                    
                    if j == 16:
                        return True        
        return True
    
    def __str__(self) -> str:
        pass


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