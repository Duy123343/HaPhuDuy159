from config import BOARD_SIZES, ENCODINGS, SOLVERS
from encoder import NQueensEncoder
from solver import NQueensSolverEngine

class BenchmarkRunner:
    def __init__(self):
        pass

    def run_all(self):
        print("=" * 70)
        print(f"{'N':<5} | {'Encoding':<20} | {'Solver':<10} | {'Status':<8} | {'Time (s)':<10}")
        print("=" * 70)

        for n in BOARD_SIZES:
            for enc_name, enc_type in ENCODINGS.items():
                for s_name in SOLVERS:
                    try:
                        encoder = NQueensEncoder(n=n, enc_type=enc_type)
                        clauses = encoder.encode()
                        engine = NQueensSolverEngine(n=n, clauses=clauses)
                        is_sat, _, duration = engine.solve(solver_name=s_name)
                        status = "SAT" if is_sat else "UNSAT"
                        print(f"{n:<5} | {enc_name:<20} | {s_name:<10} | {status:<8} | {duration:<10.4f}")
                    except Exception as e:
                        print(f"{n:<5} | {enc_name:<20} | {s_name:<10} | ERROR    | {-1.0:<10.4f}")
        print("=" * 70)
