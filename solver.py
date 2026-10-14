import time
from pysat.solvers import Solver

class NQueensSolverEngine:
    def __init__(self, n, clauses):
        self.n = n
        self.clauses = clauses

    def solve(self, solver_name='cadical'):
        start_time = time.time()
        try:
            with Solver(name=solver_name, bootstrap_with=self.clauses) as solver:
                is_sat = solver.solve()
                model = solver.get_model() if is_sat else None
        except Exception as e:
            print(f"Lỗi khi chạy solver {solver_name}: {e}")
            return False, None, -1.0
        end_time = time.time()
        duration = end_time - start_time
        return is_sat, model, duration

    def decode_board(self, model):
        if not model:
            return None
        board = [['.' for _ in range(self.n)] for _ in range(self.n)]
        max_var = self.n * self.n
        for lit in model:
            if 0 < lit <= max_var:
                idx = lit - 1
                r = idx // self.n
                c = idx % self.n
                board[r][c] = 'Q'
        return board