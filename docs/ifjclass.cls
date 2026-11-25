\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{ifjclass}[2026/11/25 IFJ documentation class]

\LoadClass[a4paper,11pt]{article}

% čeština a kódování
\RequirePackage[T1]{fontenc}
\RequirePackage[utf8]{inputenc}
\RequirePackage{lmodern}
\RequirePackage[czech]{babel}

% obrázky a logo fakulty
\RequirePackage{graphicx}
\newcommand{\logo}{%
    \includegraphics[scale=0.1,keepaspectratio]{pictures/logo_cz.png}%
}