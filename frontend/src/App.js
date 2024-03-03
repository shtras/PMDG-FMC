import { useEffect, useRef, useState } from "react";
import "./App.css";
import Row from "./components/Row";
import Keypad from "./components/Keypad";

const width = 24;
const height = 14;

function fillInitialData() {
  const res = [];
  for (let y = 0; y < height; ++y) {
    const row = [];
    for (let x = 0; x < width; ++x) {
      row.push({ id: x, text: " ", color: "yellow" });
    }
    res.push({ id: y, letters: row, sideButton: y % 2 === 0 && y >= 2 });
  }
  return res;
}

const screen_data = fillInitialData();
function App() {
  const hostname = window.location.hostname;
  const port = 3456;
  const location = "fmc";
  const uri = `http://${hostname}:${port}/${location}`;

  const [eventSource, setEventSource] = useState(null);
  const [leftRows, setLeftRows] = useState(screen_data);
  const [rightRows, setRightRows] = useState(screen_data);
  const [lastError, setLastError] = useState("");

  const [leftExec, setLeftExec] = useState(false);
  const [rightExec, setRightExec] = useState(false);

  const leftRowsRef = useRef();
  useEffect(() => {
    leftRowsRef.current = leftRows;
  }, [leftRows]);

  const rightRowsRef = useRef();
  useEffect(() => {
    rightRowsRef.current = rightRows;
  }, [rightRows]);

  const leftRowList = leftRows.map((r) => (
    <Row
      key={r.id}
      id={r.id}
      letters={r.letters}
      sideButton={r.sideButton}
      setLastError={setLastError}
      uri={uri}
      prefix="0;"
    />
  ));

  const rightRowList = rightRows.map((r) => (
    <Row
      key={r.id}
      id={r.id}
      letters={r.letters}
      sideButton={r.sideButton}
      setLastError={setLastError}
      uri={uri}
      prefix="1;"
    />
  ));

  function startEvent() {
    if (eventSource) {
      eventSource.close();
    }
    const newEventSource = new EventSource(`${uri}/server_event`);
    newEventSource.onmessage = (e) => {
      const message = JSON.parse(e.data);

      function updateRows(refRows, json) {
        return refRows.current.map((row) => {
          if (row.id in json) {
            const messageRow = json[row.id];
            return {
              ...row,
              letters: row.letters.map((letter) => {
                if (letter.id in messageRow) {
                  const messageLetter = messageRow[letter.id];
                  return {
                    ...letter,
                    text: messageLetter.text,
                    small: messageLetter.small,
                    color: messageLetter.color,
                    reverse: messageLetter.reverse,
                    unused: messageLetter.unused,
                  };
                }
                return letter;
              }),
            };
          }
          return row;
        });
      }

      if ("L" in message) {
        const newLeftRows = updateRows(leftRowsRef, message["L"]);
        setLeftRows(newLeftRows);
      }
      if ("R" in message) {
        const newLeftRows = updateRows(rightRowsRef, message["R"]);
        setRightRows(newLeftRows);
      }
      if ("leftExec" in message) {
        setLeftExec(message["leftExec"]);
      }
      if ("rightExec" in message) {
        setRightExec(message["rightExec"]);
      }
      setLastError("");
    };
    newEventSource.onerror = (e) => {
      console.log(e);
      setLastError("EventSource error");
    };
    setEventSource(newEventSource);
  }

  function stopEvent() {
    if (eventSource) {
      eventSource.close();
    }
    setEventSource(null);
  }

  function refresh() {
    fetch(`${uri}/refresh`, {
      method: "POST",
    })
      .then((response) => {
        console.log(response);
      })
      .catch((e) => {
        setLastError(e.data);
      });
  }

  function toggleDummy() {
    fetch(`${uri}/toggle_dummy`, {
      method: "POST",
    })
      .then((response) => {
        console.log(response);
      })
      .catch((e) => {
        setLastError(e.data);
      });
  }

  return (
    <>
      Error: {lastError}
      <div className="multi-fmc">
        <div className="fmc">
          <div className="screen">{leftRowList}</div>
          <Keypad prefix="0;" exec={leftExec} uri={uri} />
        </div>
        <div className="fmc">
          <div className="screen">{rightRowList}</div>
          <Keypad prefix="1;" exec={rightExec} uri={uri} />
        </div>
      </div>
      <button onClick={startEvent}>Start</button>
      <button onClick={stopEvent}>Close</button>
      <button onClick={refresh}>Refresh</button>
      <button onClick={toggleDummy}>Toggle Dummy</button>
    </>
  );
}

export default App;
