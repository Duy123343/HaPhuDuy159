from pysat.card import CardEnc

class NQueensEncoder:
    def __init__(self, n, enc_type):
        self.n = n
        self.enc_type = enc_type
        self.clauses = []
        self.top_id = n * n

    def _var(self, row, col):
        return row * self.n + col + 1

    def encode(self):
        self.clauses = []
        self.top_id = self.n * self.n
        for r in range(self.n):
            row_vars = [self._var(r, c) for c in range(self.n)]
            encoding = CardEnc.equals(
                lits=row_vars, bound=1, encoding=self.enc_type, top_id=self.top_id
            )
            self.clauses.extend(encoding.clauses)
            self.top_id = encoding.nv

        for c in range(self.n):
            col_vars = [self._var(r, c) for r in range(self.n)]
            encoding = CardEnc.equals(
                lits=col_vars, bound=1, encoding=self.enc_type, top_id=self.top_id
            )
            self.clauses.extend(encoding.clauses)
            self.top_id = encoding.nv

        for d in range(-(self.n - 1), self.n):
            diag_vars = [self._var(r, r - d) for r in range(self.n) if 0 <= r - d < self.n]
            if len(diag_vars) > 1:
                encoding = CardEnc.atmost(
                    lits=diag_vars, bound=1, encoding=self.enc_type, top_id=self.top_id
                )
                self.clauses.extend(encoding.clauses)
                self.top_id = encoding.nv

        for d in range(2 * self.n - 1):
            diag_vars = [self._var(r, d - r) for r in range(self.n) if 0 <= d - r < self.n]
            if len(diag_vars) > 1:
                encoding = CardEnc.atmost(
                    lits=diag_vars, bound=1, encoding=self.enc_type, top_id=self.top_id
                )
                self.clauses.extend(encoding.clauses)
                self.top_id = encoding.nv

        return self.clauses