from __init__ import DUTAdder


MASK_64 = (1 << 64) - 1


TEST_CASES = [
    {
        'name': 'normal add',
        'a': 0x0000000000000001,
        'b': 0x0000000000000002,
        'cin': 0,
    },
    {
        'name': 'carry out',
        'a': 0xffffffffffffffff,
        'b': 0x0000000000000000,
        'cin': 1,
    },
    {
        'name': 'high result bit',
        'a': 0x7fffffffffffffff,
        'b': 0x0000000000000001,
        'cin': 0,
    },
]


def expectedResult(a, b, cin):
    full = a + b + cin
    return full & MASK_64, (full >> 64) & 0x1


def runCase(dut, case):
    dut.a.Set(case['a'])
    dut.b.Set(case['b'])
    dut.cin.Set(case['cin'])
    dut.Step(1)

    expected_sum, expected_cout = expectedResult(case['a'], case['b'], case['cin'])
    actual_sum = dut.sum.U()
    actual_cout = dut.cout.U()
    passed = actual_sum == expected_sum and actual_cout == expected_cout

    print(
        f"{case['name']}: "
        f"a=0x{case['a']:016x} "
        f"b=0x{case['b']:016x} "
        f"cin={case['cin']} "
        f"expected_sum=0x{expected_sum:016x} "
        f"actual_sum=0x{actual_sum:016x} "
        f"expected_cout={expected_cout} "
        f"actual_cout={actual_cout} "
        f"result={'PASS' if passed else 'FAIL'}"
    )
    return passed


def main():
    dut = DUTAdder()
    try:
        results = [runCase(dut, case) for case in TEST_CASES]
    finally:
        dut.Finish()

    failed = len([result for result in results if not result])
    print(f"summary: total={len(results)} failed={failed}")
    return 1 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
