/*
 * lc_io.hpp
 *
 *  Created on: Aug 12, 2011
 *      Author: andrey
 */

#ifndef LC_IO_HPP_
#define LC_IO_HPP_

namespace long_contigs {

template<size_t k>
void LoadFromFile(std::string fileName, Graph& g,  PairedInfoIndex<Graph>& paired_index, EdgeIndex<k + 1, Graph>& index, IdTrackHandler<Graph>& conj_IntIds,
		Sequence& sequence) {

	string input_dir = cfg::get().input_dir;
	string dataset = cfg::get().dataset_name;
	string genome_filename = input_dir
			+ cfg::get().reference_genome;
	checkFileExistenceFATAL(genome_filename);
	int dataset_len = cfg::get().ds.LEN;

	typedef io::Reader<io::SingleRead> ReadStream;
	typedef io::Reader<io::PairedRead> PairedReadStream;
	typedef io::RCReaderWrapper<io::PairedRead> RCStream;

	// read data ('genome')
	std::string genome;
	{
		ReadStream genome_stream(genome_filename);
		io::SingleRead full_genome;
		genome_stream >> full_genome;
		genome = full_genome.GetSequenceString().substr(0, dataset_len); // cropped
	}
	sequence = Sequence(genome);

	INFO("Reading graph");
	scanConjugateGraph(g, conj_IntIds,fileName, paired_index);
	INFO("Graph read")
}

template<size_t k>
void AddEtalonInfo(Graph& g, EdgeIndex<k+1, Graph>& index, const Sequence& genome, PairedInfoIndices& pairedInfos) {
	size_t libCount = LC_CONFIG.read<size_t>("etalon_lib_count");

	for (size_t i = 1; i <= libCount; ++i) {
		std::string num = ToString<size_t>(i);
		size_t insertSize = LC_CONFIG.read<size_t>("etalon_insert_size_" + num);
		size_t readSize = LC_CONFIG.read<size_t>("etalon_read_size_" + num);
		INFO("Generating info with read size " << readSize << ", insert size " << insertSize);

		pairedInfos.push_back(PairedInfoIndexLibrary(readSize, insertSize, new PairedInfoIndex<Graph>(g, 0)));
		FillEtalonPairedIndex<k> (g, *pairedInfos.back().pairedInfoIndex, index, insertSize, readSize, genome);
	}
}

template<size_t k>
void AddRealInfo(Graph& g, EdgeIndex<k+1, Graph>& index, IdTrackHandler<Graph>& conj_IntIds, PairedInfoIndices& pairedInfos) {
	size_t libCount = LC_CONFIG.read<size_t>("real_lib_count");

	for (size_t i = 1; i <= libCount; ++i) {
		std::string num = ToString<size_t>(i);
		size_t insertSize = LC_CONFIG.read<size_t>("real_insert_size_" + num);
		size_t readSize = LC_CONFIG.read<size_t>("real_read_size_" + num);
		string dataset = cfg::get().dataset_name;
		pairedInfos.push_back(PairedInfoIndexLibrary(readSize, insertSize, new PairedInfoIndex<Graph>(g, 0)));

		INFO("Reading additional info with read size " << readSize << ", insert size " << insertSize);

		if (LC_CONFIG.read<bool>("real_precounted_" + num + "_" + dataset)) {
			//Reading saved paired info
			DataScanner<Graph> dataScanner(g, conj_IntIds);
			dataScanner.loadPaired(LC_CONFIG.read<string>("real_precounted_path_" + num + "_" + dataset), *pairedInfos.back().pairedInfoIndex);
		}
		else {
			//Reading paired info from fastq files
			string reads_filename1 = LC_CONFIG.read<string>("real_path_" + num + "_" + dataset + "_1");
			string reads_filename2 = LC_CONFIG.read<string>("real_path_" + num + "_" + dataset + "_2");
			checkFileExistenceFATAL(reads_filename1);
			checkFileExistenceFATAL(reads_filename2);

			typedef io::Reader<io::PairedRead> PairedReadStream;
			typedef io::RCReaderWrapper<io::PairedRead> RCStream;

			PairedReadStream pairStream(std::pair<std::string,
									  std::string>(reads_filename1,
												   reads_filename2),
												   insertSize);

			RCStream rcStream(&pairStream);


			FillPairedIndex<k, RCStream> (g, index, *pairedInfos.back().pairedInfoIndex, rcStream);
		}
		INFO("Done");
	}
}

void SavePairedInfo(Graph& g, PairedInfoIndices& pairedInfos, IdTrackHandler<Graph>& old_IDs, const std::string& fileNamePrefix) {
	INFO("Saving paired info");
	DataPrinter<Graph> dataPrinter(g, old_IDs);
	for (auto lib = pairedInfos.begin(); lib != pairedInfos.end(); ++lib) {
		std::string fileName = fileNamePrefix + "IS" + ToString(lib->insertSize) + "_RS" + ToString(lib->readSize);

		if (LC_CONFIG.read<bool>("cluster_paired_info")) {
			PairedInfoIndex<Graph> clustered_index(g);
			DistanceEstimator<Graph> estimator(g, *(lib->pairedInfoIndex), lib->insertSize, lib->readSize, cfg::get().de.delta,
					cfg::get().de.linkage_distance,
					cfg::get().de.max_distance);
			estimator.Estimate(clustered_index);

			dataPrinter.savePaired(fileName, clustered_index);
		} else {
			dataPrinter.savePaired(fileName, *(lib->pairedInfoIndex));
		}


	}
	INFO("Saved");
}

void SaveGraph(Graph& g, IdTrackHandler<Graph>& old_IDs, const std::string& fileName) {
	DataPrinter<Graph> dataPrinter(g, old_IDs);
	dataPrinter.saveGraph(fileName);
}

void DeleteAdditionalInfo(PairedInfoIndices& pairedInfos) {
	while (pairedInfos.size() > 1) {
		delete pairedInfos.back().pairedInfoIndex;
		pairedInfos.pop_back();
	}
}

void PrintPairedInfo(Graph& g, PairedInfoIndex<Graph>& pairedInfo, IdTrackHandler<Graph>& intIds) {
	for (auto iter = g.SmartEdgeBegin(); !iter.IsEnd(); ++iter) {
		for (auto iter2 = g.SmartEdgeBegin(); !iter2.IsEnd(); ++iter2) {
			omnigraph::PairedInfoIndex<Graph>::PairInfos pairs = pairedInfo.GetEdgePairInfo(*iter, *iter2);
			for (auto pair = pairs.begin(); pair != pairs.end(); ++pair) {
				INFO(intIds.ReturnIntId(pair->first) << " - " << intIds.ReturnIntId(pair->second) << " : " << pair->weight);
			}
		}
	}
}

//Convert path to contig sequence
Sequence PathToSequence(Graph& g, BidirectionalPath& path) {
	SequenceBuilder result;

	if (!path.empty()) {
		result.append(g.EdgeNucls(path[0]).Subseq(0, K));
	}
	for (auto edge = path.begin(); edge != path.end(); ++edge) {
		result.append(g.EdgeNucls(*edge).Subseq(K + 1));
	}

	return result.BuildSequence();
}

//Output
void OutputPathsAsContigs(Graph& g, std::vector<BidirectionalPath> paths, const string& filename) {
	INFO("Writing contigs");
	osequencestream oss(filename);
	for (auto path = paths.begin(); path != paths.end(); ++path ) {
		oss << PathToSequence(g, *path);
	}
	INFO("Contigs written");
}

} // namespace long_contigs

#endif /* LC_IO_HPP_ */
