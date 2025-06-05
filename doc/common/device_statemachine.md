
\begin{figure}[ht]
    \centering
    \resizebox{\linewidth}{!}{%
        \begin{tikzpicture}[
		node distance=1.6cm and 2.0cm,
		box/.style={draw, minimum height=1.2cm, minimum width=2.8cm},
		decision/.style={draw, diamond, aspect=2, text width=3cm, inner sep=1pt},
		ellipsebox/.style={draw, ellipse, minimum width=2.5cm, minimum height=1.2cm},
		arr/.style={-{Latex}, thick},
		every node/.style={align=center}
		]

		% Nodes
		\node[ellipsebox] (Overview) {Overview};
		\node[decision, below=of Overview] (TaskPrompt) {Task Selection};
		\node[ellipsebox, right=of TaskPrompt] (Focus) {Focus Mode};
		\node[box, below=of Focus] (SessionDone) {Session Complete Prompt};
		\node[ellipsebox, below=of SessionDone] (Break) {Break Mode};
		\node[box, below=of Break] (NextChoice) {Next task prompt};

		% Connections (orthogonal style)
		\draw[arr] (Overview) -- node[left] {Timeblock starts} (TaskPrompt);

		\draw[arr] (Overview.east) -- ++(1.2,0) coordinate (toFocus1) node[above] {Task chosen}
		-- (toFocus1 |- Focus) -- (Focus.west);

		\draw[arr] (TaskPrompt) -- node[below] {Task is chosen} (Focus);

		\draw[arr] (Focus) -- node[left] {Complete / Conclude} (SessionDone);

		\draw[arr] (Focus.east) -- ++(1.5,0) coordinate (toSDRight)
		-- (toSDRight |- SessionDone) node[right] {Timeout} -- (SessionDone.east);

		\draw[arr] (SessionDone) -- node[right] {Inside of timeblock} (Break);

		\draw[arr] (SessionDone.south) -- ++(0,-0.2) -- ++(-8.2,0) coordinate (backUp1) node[below] {Outside of timeblock}
		-- (backUp1 |- Overview) -- ++(0,1.5) coordinate (backUp2)
		-- (Overview |- backUp2) -- (Overview.north);

		\draw[arr] (Break) -- node[right] {Timeout} (NextChoice);

		\draw[arr] (NextChoice.east) -- ++(3,0) coordinate (toFocus2) -- ++(0, 4) node[right] {Continue}
		-- (toFocus2 |- Focus.north) -- ++(0,1) coordinate (toFocus3) -- (Focus.north |- toFocus3)
		-- (Focus.north);

		\draw[arr] (NextChoice.west) -- ++(-4,0) coordinate (toPrompt) node[below] {Complete / Conclude}
		-- (TaskPrompt |- toPrompt) -- (TaskPrompt.south);
        \end{tikzpicture}
    }
    \caption{Flow of task focus and selection during a timeblock session}
    \label{fig:task-flow}
\end{figure}