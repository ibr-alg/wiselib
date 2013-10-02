#!/bin/bash
algo=$1
topologies=$2
echo " 
\documentclass[9pt,a4paper]{article}
\usepackage{graphicx}
\marginparwidth = 0pt
\textwidth = 500pt


\begin{document}

" > $algo.$topologies.report.tex
echo "\section{$algo clustering algorithm, $topologies topologies}" >> $algo.$topologies.report.tex
for file in data/*.$topologies.$algo.dat.avg
do
	
	tval=${file#*/}
	dval=${tval%%.*}
	echo "\paragraph*{d=$dval} Data: \\\\" >> $algo.$topologies.report.tex
	awk '
	BEGIN { print "\\begin{tabular}{|r|r|r|r|r|r|r|}";
		print "\\hline";}
	{
		if ($1 ~ "#"){
			print " Total nodes & Clusters & Simple Nodes & Shawn Rounds & Time & Avg. Size & Avg. Diameter \\\\ ";
		}
		else{
			print $1" & "$2" & "$3" & "$4" & "$5" & "$7" & "$6" \\\\ ";
		}
	  print "\\hline";
	} 
	END { print "\\end{tabular}"; }
	' $file >> $algo.$topologies.report.tex

	echo "\paragraph*{d=$dval} Messages: \\\\" >> $algo.$topologies.report.tex
	awk '
	BEGIN { print "\n";
		print "\\begin{tabular}{|r|r|r|r|r|r|}";
		print "\\hline";}
	{
		if ($1 ~ "#"){
			print " Total nodes & Total Messages & Join & Join Accept & Join Deny & Resume\\\\ ";
		}
		else{
			print $1" & "$8" & "$9" & "$10" & "$11" & "$12" \\\\ ";
		}
	  print "\\hline";
	} 
	END { print "\\end{tabular}"; }

	' $file >> $algo.$topologies.report.tex

done
	echo "\paragraph*{} Plots: \\\\" >> $algo.$topologies.report.tex
for file in plots/*"_"$algo"_"$topologies".png"
do

	echo " \includegraphics[scale=0.5,keepaspectratio=true]{$file} \\\\ " >> $algo.$topologies.report.tex

done







echo " 
\end{document}


" >> $algo.$topologies.report.tex

pdflatex -interaction=nonstopmode $algo.$topologies.report.tex > /dev/null

rm $algo.$topologies.report.aux $algo.$topologies.report.log -f
