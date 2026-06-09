/*
 * fleet_agent.c — c-ternary Fleet Integration Demo
 *
 * Demonstrates agent-to-agent ternary logic communication using the
 * c-ternary.h vector ops and I2I wire packing.
 *
 * Two agents (A and B) each have a stance vector on 8 questions.
 * The demo computes:
 *   - Consensus  (element-wise AND — pessimistic agreement)
 *   - Disagreement (element-wise XOR — where they differ)
 *   - Trust score (normalized dot product)
 *   - I2I wire format (pack consensus into transport bytes)
 *
 * Build:  make
 * Usage:  ./fleet_agent
 *
 * SPDX-License-Identifier: MIT
 */

#define C_TERNARY_IMPL
#include "c-ternary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Pretty-print helpers                                              */
/* ------------------------------------------------------------------ */

/** Print a trit vector as a human-readable string like "[ - 0 + ...]" */
static void print_trits(const ct_trit_t *vec, int n)
{
    putchar('[');
    for (int i = 0; i < n; i++) {
        printf(" %c", ct_to_char(vec[i]));
    }
    printf(" ]");
}

/** Print raw bytes as lowercase hex (for I2I wire format). */
static void print_hex(const uint8_t *data, int len)
{
    for (int i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

/* ------------------------------------------------------------------ */
/*  Main demo                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== c-ternary Fleet Integration Demo ===\n\n");
    printf("Platform: ARM64 / GCC C99\n\n");

    /* ------------------------------------------------------------------ */
    /*  Define agent stances on 8 coordination questions                    */
    /* ------------------------------------------------------------------ */
    /*
     * Question mapping (fictional):
     *   0: "Prioritize speed over safety?"
     *   1: "Use external dependencies?"
     *   2: "Support legacy protocol?"
     *   3: "Adopt hot-reload?"
     *   4: "Implement audit logging?"
     *   5: "Enable real-time sync?"
     *   6: "Use microservices?"
     *   7: "Open-source the core?"
     */

    enum { N_QUESTIONS = 8 };

    ct_trit_t agent_a[N_QUESTIONS] = {
        CT_TRIT_NEG,   /* 0: speed over safety?        — No           */
        CT_TRIT_ZERO,  /* 1: external dependencies?     — Neutral      */
        CT_TRIT_POS,   /* 2: support legacy protocol?  — Yes          */
        CT_TRIT_ZERO,  /* 3: hot-reload?               — Neutral      */
        CT_TRIT_NEG,   /* 4: audit logging?            — No           */
        CT_TRIT_POS,   /* 5: real-time sync?           — Yes          */
        CT_TRIT_ZERO,  /* 6: microservices?            — Neutral      */
        CT_TRIT_NEG    /* 7: open-source core?         — No           */
    };

    ct_trit_t agent_b[N_QUESTIONS] = {
        CT_TRIT_POS,   /* 0: speed over safety?        — Yes          */
        CT_TRIT_POS,   /* 1: external dependencies?     — Yes          */
        CT_TRIT_ZERO,  /* 2: support legacy protocol?  — Neutral      */
        CT_TRIT_NEG,   /* 3: hot-reload?               — No           */
        CT_TRIT_ZERO,  /* 4: audit logging?            — Neutral      */
        CT_TRIT_POS,   /* 5: real-time sync?           — Yes          */
        CT_TRIT_NEG,   /* 6: microservices?            — No           */
        CT_TRIT_POS    /* 7: open-source core?         — Yes          */
    };

    /* ------------------------------------------------------------------ */
    /*  Print stances                                                     */
    /* ------------------------------------------------------------------ */

    printf("Agent A stance: ");
    print_trits(agent_a, N_QUESTIONS);
    printf("\n");

    printf("Agent B stance: ");
    print_trits(agent_b, N_QUESTIONS);
    printf("\n\n");

    /* ------------------------------------------------------------------ */
    /*  1. Consensus (AND) — pessimistic agreement                        */
    /* ------------------------------------------------------------------ */

    ct_trit_t consensus[N_QUESTIONS];
    ct_and_vec(agent_a, agent_b, consensus, N_QUESTIONS);
    printf("Consensus (AND):  ");
    print_trits(consensus, N_QUESTIONS);
    printf("\n");

    /* ------------------------------------------------------------------ */
    /*  2. Disagreement (XOR) — where stances diverge                     */
    /* ------------------------------------------------------------------ */

    ct_trit_t disagreement[N_QUESTIONS];
    for (int i = 0; i < N_QUESTIONS; i++) {
        disagreement[i] = ct_xor(agent_a[i], agent_b[i]);
    }
    printf("Disagreement (XOR): ");
    print_trits(disagreement, N_QUESTIONS);
    printf("\n\n");

    /* ------------------------------------------------------------------ */
    /*  3. Trust score (normalized dot product)                           */
    /* ------------------------------------------------------------------ */

    /*
     * Trust = how aligned the agents are.
     * dot = Σ a[i] * b[i] ∈ [-N, N] since each product ∈ {-1, 0, +1}
     * Normalize to [0,1]: trust = (dot + N) / (2*N)
     *   1.0 = perfect agreement, 0.5 = orthogonal, 0.0 = perfect opposition
     */
    int dot = ct_dot(agent_a, agent_b, N_QUESTIONS);
    double trust = (double)(dot + N_QUESTIONS) / (double)(2 * N_QUESTIONS);
    printf("Raw dot product: %d  (max=%d, min=%d)\n", dot, N_QUESTIONS, -N_QUESTIONS);
    printf("Trust score:     %.4f  (1.0=perfect, 0.5=random, 0.0=opposed)\n\n", trust);

    /* ------------------------------------------------------------------ */
    /*  4. I2I Packing — encode consensus into wire format                */
    /* ------------------------------------------------------------------ */

    int payload_size = ct_i2i_payload_size(N_QUESTIONS);
    int total_size   = CT_I2I_HEADER_SIZE + payload_size;
    uint8_t *wire    = (uint8_t *)malloc(total_size);
    if (!wire) {
        fprintf(stderr, "ERROR: malloc failed\n");
        return 1;
    }

    /* Zero the buffer before packing (payload bits OR into zeroed region) */
    memset(wire, 0, total_size);

    int written = ct_pack_trits(consensus, N_QUESTIONS, wire, total_size);
    if (written < 0) {
        fprintf(stderr, "ERROR: ct_pack_trits failed\n");
        free(wire);
        return 1;
    }

    printf("I2I wire format (%d bytes):\n", written);
    printf("  Version: %d\n", wire[0]);
    printf("  Trit count: %d\n", N_QUESTIONS);
    printf("  Payload hex: ");
    print_hex(wire + CT_I2I_HEADER_SIZE, payload_size);
    printf("\n  Full packet hex: ");
    print_hex(wire, written);
    printf("\n\n");

    /* ------------------------------------------------------------------ */
    /*  5. Round-trip verification — unpack back                          */
    /* ------------------------------------------------------------------ */

    ct_trit_t unpacked[N_QUESTIONS];
    int n_unpacked = ct_unpack_trits(wire, written, unpacked, N_QUESTIONS);
    if (n_unpacked < 0) {
        fprintf(stderr, "ERROR: ct_unpack_trits failed\n");
        free(wire);
        return 1;
    }

    printf("Round-trip verification:\n");
    printf("  Unpacked consensus: ");
    print_trits(unpacked, n_unpacked);
    printf("\n");
    int match = (n_unpacked == N_QUESTIONS
                 && memcmp(consensus, unpacked, N_QUESTIONS) == 0);
    printf("  Vectors match: %s\n\n", match ? "YES ✓" : "NO ✗");

    /* ------------------------------------------------------------------ */
    /*  6. Additional metrics (for fleet coordination insight)            */
    /* ------------------------------------------------------------------ */

    int distance = ct_distance(agent_a, agent_b, N_QUESTIONS);
    int norm_a   = ct_norm(agent_a, N_QUESTIONS);
    int norm_b   = ct_norm(agent_b, N_QUESTIONS);
    int sum_a    = ct_sum(agent_a, N_QUESTIONS);
    int sum_b    = ct_sum(agent_b, N_QUESTIONS);

    printf("Coordination metrics:\n");
    printf("  Hamming distance:     %d/%d (%.2f%%)\n",
           distance, N_QUESTIONS,
           100.0 * distance / N_QUESTIONS);
    printf("  Agent A conviction:   %+.1f  (sum=%d/%d non-zero)\n",
           (double)sum_a, sum_a, norm_a);
    printf("  Agent B conviction:   %+.1f  (sum=%d/%d non-zero)\n",
           (double)sum_b, sum_b, norm_b);
    printf("\n");
    printf("=== Demo complete ===\n");

    free(wire);
    return 0;
}
