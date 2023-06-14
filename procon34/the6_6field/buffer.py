import collections

import numpy as np

import othello


class ReplayBuffer:

    #初期化
    def __init__(self, buffer_size):

        self.buffer = collections.deque(maxlen=buffer_size)#dequeでは先頭・末尾の要素を追加・削除するappend(), appendleft(), pop(), popleft()がすべてO(1)で実行できる。maxlenつまり、要素の最大値をbuffer_sizeで設定している

    #bufferの要素数を返す
    def __len__(self): 
        return len(self.buffer)

    def get_minibatch(self, batch_size):

        #バッファから取得する情報をbatch_size個ランダムに選ぶ
        indices = np.random.choice(range(len(self.buffer)), size=batch_size)#第一引数はリスト、第二引数は作成するリストの大きさ、第3引数に第一引数のリストの各要素を取り出す確率を入れることができる

        #選んだ情報をsamplesにリストで代入
        samples = [self.buffer[idx] for idx in indices]

        #受け取ったオセロの情報を２次元で代入
        states = np.stack(
            [othello.encode_state(s.state, s.player) for s in samples],
            axis=0)

        mcts_policy = np.array(
            [s.mcts_policy for s in samples], dtype=np.float32)

        rewards = np.array(
            [s.reward for s in samples], dtype=np.float32).reshape(-1, 1)

        return (states, mcts_policy, rewards)

    #bufferに新しい情報を追加する
    def add_record(self, record):
        for sample in record:
            self.buffer.append(sample)
