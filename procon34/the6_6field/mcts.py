import math
import random
import json

import numpy as np
import tensorflow as tf

import othello


class MCTS:
    #初期化
    def __init__(self, network, alpha, c_puct=1.0, epsilon=0.25):

        self.network = network

        self.alpha = alpha

        #PUCT (Polynomial Upper Confidence Tree)
        #詳細 https://ja.wikipedia.org/wiki/%E3%83%A2%E3%83%B3%E3%83%86%E3%82%AB%E3%83%AB%E3%83%AD%E6%9C%A8%E6%8E%A2%E7%B4%A2#PUCT_(Polynomial_Upper_Confidence_Tree)
        self.c_puct = c_puct

        self.eps = epsilon

        #: prior probability
        #事前確率（じぜんかくりつ、英: prior probability）とは条件付き確率の一種で、証拠がない条件で、ある変数について知られていることを確率として表現するものである
        self.P = {}

        #: visit count
        self.N = {}

        #: W is total action-value and Q is mean action-value
        #Qは行動価値関数を表し、Wは行動価値関数の合計である
        self.W = {}

        #: cache next states to save computation
        #計算の保存するための次の状態のキャッシュ
        self.next_states = {}

        #: string is hashable
        #hashable というのはハッシュ値の使用ができるという意味
        self.state_to_str = (
            #json.dumps データをJSON形式にエンコードすることのできる関数
            #状態をJSON形式にエンコードしたものとString型のプレイヤーを代入する関数
            lambda state, player: json.dumps(state) + str(player)
            )

    def search(self, root_state, current_player, num_simulations):

        s = self.state_to_str(root_state, current_player)

        if s not in self.P:
            _ = self._expand(root_state, current_player)

        valid_actions = othello.get_valid_actions(root_state, current_player)

        #: Adding Dirichlet noise to the prior probabilities in the root node
        if self.alpha is not None:
            dirichlet_noise = np.random.dirichlet(alpha=[self.alpha]*len(valid_actions))
            for a, noise in zip(valid_actions, dirichlet_noise):
                self.P[s][a] = (1 - self.eps) * self.P[s][a] + self.eps * noise

        #: MCTS simulation
        for _ in range(num_simulations):

            U = [self.c_puct * self.P[s][a] * math.sqrt(sum(self.N[s])) / (1 + self.N[s][a])
                for a in range(othello.ACTION_SPACE)]
            Q = [w / n if n != 0 else 0 for w, n in zip(self.W[s], self.N[s])]

            assert len(U) == len(Q) == othello.ACTION_SPACE

            scores = [u + q for u, q in zip(U, Q)]

            #: Mask invalid actions
            scores = np.array([score if action in valid_actions else -np.inf
                                for action, score in enumerate(scores)])

            #: np.argmaxでは同値maxで偏るため
            action = random.choice(np.where(scores == scores.max())[0])

            next_state = self.next_states[s][action]

            v = -self._evaluate(next_state, -current_player)

            self.W[s][action] += v

            self.N[s][action] += 1

        mcts_policy = [n / sum(self.N[s]) for n in self.N[s]]

        return mcts_policy

    def _expand(self, state, current_player):

        s = self.state_to_str(state, current_player)

        #マシンのCPUを使用する
        with tf.device("/cpu:0"):
            #方策と価値をネットワーク関数で導出する
            nn_policy, nn_value = self.network.predict(
                othello.encode_state(state, current_player))

        nn_policy, nn_value = nn_policy.numpy().tolist()[0], nn_value.numpy()[0][0]

        #状態sの時の方策を代入
        self.P[s] = nn_policy
        
        self.N[s] = [0] * othello.ACTION_SPACE
        self.W[s] = [0] * othello.ACTION_SPACE

        #有効な行動を取得
        valid_actions = othello.get_valid_actions(state, current_player)

        #: cache valid actions and next state to save computation
        #有効な行動と次の状態のキャッシュして計算を保存する
        self.next_states[s] = [
            #三項演算子 (真の時実行) if(条件式) else (偽の時実行)
            othello.step(state, action, current_player)[0] if (action in valid_actions) else None
            for action in range(othello.ACTION_SPACE)]

        return nn_value

    def _evaluate(self, state, current_player):

        s = self.state_to_str(state, current_player)

        if othello.is_done(state, current_player):
            #: ゲーム終了
            #一人目のスコアと二人目のスコアを出す 勝ち:1 負け:-1 引き分け:0
            reward_first, reward_second = othello.get_result(state)
            reward = reward_first if current_player == 1 else reward_second
            return reward

        elif s not in self.P:
            #: ゲーム終了していないリーフノードの場合は展開
            nn_value = self._expand(state, current_player)
            return nn_value

        else:
            #: 子ノードをevaluate
            U = [self.c_puct * self.P[s][a] * math.sqrt(sum(self.N[s])) / (1 + self.N[s][a])
                for a in range(othello.ACTION_SPACE)]
            Q = [q / n if n != 0 else q for q, n in zip(self.W[s], self.N[s])]

            assert len(U) == len(Q) == othello.ACTION_SPACE

            valid_actions = othello.get_valid_actions(state, current_player)

            scores = [u + q for u, q in zip(U, Q)]
            scores = np.array([score if action in valid_actions else -np.inf
                                for action, score in enumerate(scores)])

            #np.where(条件式)条件を満たすインデックスのndarrayのタプルを返す。
            best_action = random.choice(np.where(scores == scores.max())[0])

            next_state = self.next_states[s][best_action]

            v = -self._evaluate(next_state, -current_player)

            self.W[s][best_action] += v
            self.N[s][best_action] += 1

            return v
