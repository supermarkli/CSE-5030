#include <cpu/bpred2.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

BranchPredictor bpred2_state;
int g_bpred2_mode = 0;

static uint32_t spec_lht[LHT_SIZE];
static const uint32_t LOCAL_HIST_MASK = 0x3;

static inline uint32_t bpred2_pht_index(uint32_t hist, vaddr_t pc) {
    uint32_t hist2 = hist & LOCAL_HIST_MASK;
    uint32_t pci   = (pc >> 2) & (LHT_SIZE - 1);
    return (hist2 << 10) | pci;
}

BPredResult bpred2_predict(BranchPredictor *bp, vaddr_t pc) {
    BPredResult r = {0, 0, 0};

    uint32_t bi  = (pc >> 2) & (BTB_SIZE - 1);
    uint32_t tag = pc >> 11;
    r.btb_hit = bp->btb[bi].valid && (bp->btb[bi].tag == tag);
    r.target  = r.btb_hit ? bp->btb[bi].target : 0;

    uint32_t li        = (pc >> 2) & (LHT_SIZE - 1);
    uint32_t spec_hist = spec_lht[li];
    uint32_t phti      = bpred2_pht_index(spec_hist, pc);
    r.taken            = (bp->gpht[phti] >= 2);

    spec_lht[li] = ((spec_hist << 1) | r.taken) & ((1 << LHT_BITS) - 1);

    return r;
}

void bpred2_update(BranchPredictor *bp, IR_Inst *ir) {
    if (ir->type != ITYPE_BRANCH && ir->type != ITYPE_JAL && ir->type != ITYPE_JALR)
        return;

    vaddr_t pc      = ir->pc;
    int     actual  = (ir->dnpc != ir->snpc);
    int     mispred = (ir->dnpc != ir->bp_predicted_pc);

    bp->predictions++;
    if (mispred) bp->mispredictions++;

    uint32_t bi  = (pc >> 2) & (BTB_SIZE - 1);
    uint32_t tag = pc >> 11;
    if (actual) {
        if (!bp->btb[bi].valid || bp->btb[bi].tag != tag) bp->btb_misses++;
        bp->btb[bi].valid  = 1;
        bp->btb[bi].tag    = tag;
        bp->btb[bi].target = ir->dnpc;
    }

    uint32_t li   = (pc >> 2) & (LHT_SIZE - 1);
    uint32_t hist = bp->lht[li];
    uint32_t phti = bpred2_pht_index(hist, pc);
    if (actual  && bp->gpht[phti] < 3) bp->gpht[phti]++;
    if (!actual && bp->gpht[phti] > 0) bp->gpht[phti]--;
    bp->lht[li] = ((hist << 1) | actual) & ((1 << LHT_BITS) - 1);

    if (mispred)
        spec_lht[li] = bp->lht[li];

    bp->ghr = ((bp->ghr << 1) | actual) & ((1 << GHR_BITS) - 1);
}

void bpred2_report(BranchPredictor *bp) {
    printf("=== Branch Prediction Statistics (bpred2) ===\n");
    printf("Predictions     : %" PRIu64 "\n", bp->predictions);
    printf("Mispredictions  : %" PRIu64 "\n", bp->mispredictions);
    if (bp->predictions > 0)
        printf("Accuracy        : %.2f%%\n",
               100.0 * (bp->predictions - bp->mispredictions) / bp->predictions);
    printf("BTB cold misses : %" PRIu64 "\n", bp->btb_misses);
    printf("=============================================\n");
}
