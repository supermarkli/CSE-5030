#!/usr/bin/env python3
"""
riscv_moesi_riscv.py
RISC-V MOESI Cache Coherence Configuration (Timing CPU)
This configuration uses TimingSimpleCPU to demonstrate realistic cache coherence activity
"""

import argparse
import os
import sys
import shlex

import m5
from m5.objects import *
from m5.defines import buildEnv
from m5.util import addToPath

config_path = os.path.dirname(os.path.abspath(__file__))
configs_dir = os.path.abspath("../configs")

# Ensure configs directory is in sys.path
if configs_dir not in sys.path:
    sys.path.insert(0, configs_dir)

# Use addToPath to add configs directory (points to gem5 root directory)
addToPath(configs_dir)

# Verify path setup
ruby_module_path = os.path.join(configs_dir, 'ruby', 'Ruby.py')
if not os.path.exists(ruby_module_path):
    print(f"ERROR: Cannot find Ruby module at: {ruby_module_path}")
    print(f"Config path: {config_path}")
    print(f"Configs dir: {configs_dir}")
    print(f"Python sys.path:")
    for p in sys.path[:5]:
        print(f"  {p}")
    sys.exit(1)

# Import Ruby module - it will automatically use the compile-time protocol
from ruby.Ruby import create_system, define_options


def main():
    parser = argparse.ArgumentParser(
        description="RISC-V MOESI Cache Coherence with Timing CPU"
    )

    # Add basic options
    parser.add_argument(
        '--num-cpus', type=int, default=2,
        help='Number of CPU cores (default: 2)'
    )
    parser.add_argument(
        '--benchmark', type=str, required=True,
        help='RISC-V binary to execute (local path)'
    )
    parser.add_argument(
        '--mem-size', type=str, default='32GiB',
        help='Memory size (default: 32GiB, matches DRAM capacity)'
    )
    parser.add_argument(
        '--sys-clock', type=str, default='1GHz',
        help='System clock (default: 1GHz)'
    )
    parser.add_argument(
        '--sys-voltage', type=str, default='1.0V',
        help='System voltage (default: 1.0V)'
    )
    parser.add_argument(
        '--benchmark-args', type=str, default='',
        help='Arguments for benchmark (e.g., "1000")'
    )

    # Add all Ruby-related options
    define_options(parser)

    # Set some default values
    parser.set_defaults(
        cpu_type='TimingSimpleCPU',  # Use RISC-V TimingSimpleCPU
        ruby_clock='1GHz',
        network='simple',
        topology='Crossbar',
        num_dirs=1,
        num_l2caches=2,  # MOESI_CMP_directory protocol requires this parameter
        mem_type='DDR4_2400_16x4',  # Use correct memory type (16x4 configuration)
        enable_dram_powerdown=False,  # DRAM power management option
        l1i_size='32KiB',
        l1d_size='32KiB',
        l2_size='256KiB',
        l1i_assoc=4,
        l1d_assoc=4,
        l2_assoc=8,
        num_l2_banks=0,  # Will be set to num_cpus later
        cacheline_size=64,
        ports=4,
        protocol='MOESI_CMP_directory',
    )

    args = parser.parse_args()

    bench_args = shlex.split(args.benchmark_args) if args.benchmark_args else []

    # Set num_l2_banks default value
    if args.num_l2_banks == 0:
        args.num_l2_banks = args.num_cpus

    # Set num_l2caches (required by MOESI_CMP_directory protocol)
    if not hasattr(args, 'num_l2caches') or args.num_l2caches is None:
        args.num_l2caches = args.num_cpus

    # Display configuration information
    protocol = buildEnv.get('PROTOCOL', 'Unknown')
    print(f"\n{'='*60}")
    print("RISC-V MOESI Cache Coherence Simulation")
    print(f"{'='*60}")
    print(f"CPU Cores: {args.num_cpus}")
    print(f"Binary: {args.benchmark}")
    print(f"Protocol: {protocol}")
    print(f"CPU Type: TimingSimpleCPU")
    print(f"L1I Cache: {args.l1i_size}, {args.l1i_assoc}-way")
    print(f"L1D Cache: {args.l1d_size}, {args.l1d_assoc}-way")
    print(f"L2 Cache: {args.l2_size}, {args.l2_assoc}-way")
    print(f"Network: {args.network}")
    print(f"Topology: {args.topology}")
    print(f"{'='*60}\n")

    # Create system
    system = System()
    system.voltage_domain = VoltageDomain(voltage=args.sys_voltage)
    system.clk_domain = SrcClockDomain(
        clock=args.sys_clock,
        voltage_domain=system.voltage_domain
    )
    system.mem_ranges = [AddrRange(args.mem_size)]
    system.mem_mode = 'timing'

    # Create TimingSimpleCPU instances
    system.cpu = [TimingSimpleCPU(cpu_id=i) for i in range(args.num_cpus)]

    # Set interrupt controllers and clock domains
    for cpu in system.cpu:
        cpu.createInterruptController()
        cpu.clk_domain = system.clk_domain

    # Create Ruby system (automatically uses compile-time protocol)
    create_system(
        options=args,
        full_system=False,
        system=system,
        dma_ports=[],
        bootmem=None,
        cpus=system.cpu
    )

    # Set Ruby clock domain
    system.ruby.clk_domain = SrcClockDomain(
        clock=args.ruby_clock,
        voltage_domain=system.voltage_domain
    )

    # Connect CPU ports to Ruby
    for i, ruby_port in enumerate(system.ruby._cpu_ports):
        if ruby_port.support_data_reqs and ruby_port.support_inst_reqs:
            system.cpu[i].icache_port = ruby_port.in_ports
            system.cpu[i].dcache_port = ruby_port.in_ports
        elif ruby_port.support_data_reqs:
            system.cpu[i].dcache_port = ruby_port.in_ports
        elif ruby_port.support_inst_reqs:
            system.cpu[i].icache_port = ruby_port.in_ports

    # Set workload
    if args.benchmark:
        if args.benchmark.startswith("/") or args.benchmark.startswith("./"):
            abs_path = os.path.abspath(args.benchmark)
            process = Process()
            process.cmd = [abs_path] + bench_args

            for cpu in system.cpu:
                cpu.workload = process
                cpu.createThreads()

            system.workload = SEWorkload.init_compatible(abs_path)

    # Create root object
    root = Root(full_system=False, system=system)

    # Set global frequency
    m5.ticks.setGlobalFrequency('1THz')

    # Instantiate simulation
    print("Starting simulation instantiation...")
    m5.instantiate()

    # Run simulation
    print("Starting simulation...")
    exit_event = m5.simulate()

    print(f"\n{'='*60}")
    print("Simulation Complete!")
    print(f"{'='*60}")
    print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
    print(f"\nResults can be found in m5out/")
    print(f"\nKey MOESI Cache Coherence Statistics:")
    print(f"  To view all Ruby statistics:")
    print(f"    cat m5out/stats.txt | grep -E 'ruby'")
    print(f"\n")


if __name__ == "__m5_main__":
    main()