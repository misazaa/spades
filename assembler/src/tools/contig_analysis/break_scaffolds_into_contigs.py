#!/usr/bin/python

############################################################################
# Copyright (c) 2011-2013 Saint-Petersburg Academic University
# All Rights Reserved
# See file LICENSE for details.
############################################################################


# Convert contigs (i.e a reference) for experiment of running SPAdes on E. coli MC reads in "IonTorrent" mode
# (all series of repeated nucleotides are changed to single nucleotides).

import sys
import os

sys.path.append(os.path.join(os.path.abspath(sys.path[0]), '../libs'))

import fastaparser

# MAIN
def break_scaffolds(argv):
    if (len(argv) != 4) and (len(argv) != 2):
        print("Usage: " + argv[0] + " <input fasta (scaffolds)> (to get stats on sizes of Ns regions)")	
        print("Usage: " + argv[0] + " <input fasta (scaffolds)> <THRESHOLD> <output fasta (contigs)> (to break contigs on Ns regions of size >= THRESHOLD)")	
        sys.exit()

    BREAK_SCAFFOLDS = False
    if len(argv) == 4:
        BREAK_SCAFFOLDS = True

    N_NUMBER = None
    counter = 0
    if BREAK_SCAFFOLDS:
        N_NUMBER = int(argv[2])

    sizes_of_Ns_regions = dict()
    new_fasta = []
    for id, (name, seq) in enumerate(fastaparser.read_fasta(argv[1])): 
        i = 0
        cur_contig_number = 1
        cur_contig_start = 0
        while (i < len(seq)) and (seq.find("N", i) != -1):
            start = seq.find("N", i)
            end = start + 1
            while (end != len(seq)) and (seq[end] == 'N'):
                end += 1        

            i = end + 1
            if BREAK_SCAFFOLDS and (end - start) >= N_NUMBER:
                new_fasta.append((name.split()[0] + "_" + str(cur_contig_number), seq[cur_contig_start:start]))
                cur_contig_number += 1
                cur_contig_start = end

            if not BREAK_SCAFFOLDS:
                if (end - start) in sizes_of_Ns_regions:
                    sizes_of_Ns_regions[(end - start)] += 1
                else:
                    sizes_of_Ns_regions[(end - start)] = 1

        if BREAK_SCAFFOLDS:
            new_fasta.append((name.split()[0] + "_" + str(cur_contig_number), seq[cur_contig_start:]))
            counter += cur_contig_number  

    if BREAK_SCAFFOLDS:
        fastaparser.write_fasta_to_file(argv[3], new_fasta)
        #print (" * " + str(id + 1) + " scaffold(s) were broken into " + str(counter) + " contig(s)")
    else:
        list_of_sizes = sizes_of_Ns_regions.keys()
        list_of_sizes.sort()
        for k in list_of_sizes:
            print k, sizes_of_Ns_regions[k]


