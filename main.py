import argparse
from encoder import NQueensEncoder
from solver import NQueensSolverEngine
from benchmark import BenchmarkRunner
from pysat.card import EncType

def main():
    parser = argparse.ArgumentParser(description="Hệ thống giải quyết bài toán N-Queens bằng SAT Solving")
    parser.add_argument("-n", type=int, default=8, help="Kích thước bàn cờ N (mặc định: 8)")
    parser.add_argument("--solver", type=str, default="cadical", help="Tên SAT solver ('cadical', 'g3')")
    parser.add_argument("--benchmark", action="store_true", help="Chạy đánh giá thực nghiệm toàn diện")
    args = parser.parse_args()

    if args.benchmark:
        print("Đang tiến hành chạy Benchmark thực nghiệm với các kích thước lớn...")
        runner = BenchmarkRunner()
        runner.run_all()
    else:
        print(f"Đang giải bài toán {args.n}-Queens sử dụng {args.solver}...")
        encoder = NQueensEncoder(n=args.n, enc_type=EncType.seqcounter)
        clauses = encoder.encode()

        engine = NQueensSolverEngine(n=args.n, clauses=clauses)
        is_sat, model, duration = engine.solve(solver_name=args.solver)

        if is_sat:
            print(f"Thành công! Tìm thấy lời giải trong {duration:.4f} giây.")
            board = engine.decode_board(model)
            for row in board:
                print(" ".join(row))
        else:
            print("Không tìm thấy lời giải hoặc bài toán vô nghiệm.")

if __name__ == "__main__":
    main()