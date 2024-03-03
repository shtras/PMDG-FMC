import click from "../sounds/click.wav";

function keyClick(keyId, uri) {
  const audio = new Audio(click);
  audio.play();
  fetch(`${uri}/button_click`, {
    method: "POST",
    headers: {
      "Content-Type": "text/plain", // Specify the content type you are sending
    },
    body: keyId,
  })
    .then((response) => {
      console.log(response);
    })
    .catch((e) => {
      console.log(e);
    });
}

function FunctionKey(props) {
  const lines = props.text.map((l) => (
    <>
      {l}
      <br />
    </>
  ));
  return (
    <button
      className="function-key"
      onClick={(e) => keyClick(props.prefix + props.text, props.uri)}
    >
      {lines}
    </button>
  );
}

function ExecLight(props) {
  let className = "exec-light";
  if (props.on) {
    className += " exec-light-on";
  } else {
    className += " exec-light-off";
  }
  return <div className={className}></div>;
}

function UpperKeys(props) {
  return (
    <>
      <div className="upper-keys">
        <FunctionKey
          prefix={props.prefix}
          uri={props.uri}
          text={["INIT", "REF"]}
        />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["RTE"]} />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["CLB"]} />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["CRZ"]} />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["DES"]} />
        <ExecLight on={props.exec} />
      </div>
      <br />
      <div className="upper-keys">
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["MENU"]} />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["LEGS"]} />
        <FunctionKey
          prefix={props.prefix}
          uri={props.uri}
          text={["DEP", "ARR"]}
        />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["HOLD"]} />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["PROG"]} />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["EXEC"]} />
      </div>
    </>
  );
}

function LowerKeys(props) {
  return (
    <>
      <div className="upper-keys">
        <FunctionKey
          prefix={props.prefix}
          uri={props.uri}
          text={["N1", "LIMIT"]}
        />
        <FunctionKey prefix={props.prefix} uri={props.uri} text={["FIX"]} />
      </div>
      <div className="upper-keys">
        <FunctionKey
          prefix={props.prefix}
          uri={props.uri}
          text={["PREV", "PAGE"]}
        />
        <FunctionKey
          prefix={props.prefix}
          uri={props.uri}
          text={["NEXT", "PAGE"]}
        />
      </div>
    </>
  );
}

function NumberKey(props) {
  return (
    <button
      className="digit-key"
      onClick={(e) => keyClick(props.prefix + props.text, props.uri)}
    >
      {props.text}
    </button>
  );
}

function NumberKeys(props) {
  return (
    <>
      <div className="digit-keys">
        <NumberKey text="1" uri={props.uri} prefix={props.prefix} />
        <NumberKey text="2" uri={props.uri} prefix={props.prefix} />
        <NumberKey text="3" uri={props.uri} prefix={props.prefix} />
      </div>
      <div className="digit-keys">
        <NumberKey text="4" uri={props.uri} prefix={props.prefix} />
        <NumberKey text="5" uri={props.uri} prefix={props.prefix} />
        <NumberKey text="6" uri={props.uri} prefix={props.prefix} />
      </div>
      <div className="digit-keys">
        <NumberKey text="7" uri={props.uri} prefix={props.prefix} />
        <NumberKey text="8" uri={props.uri} prefix={props.prefix} />
        <NumberKey text="9" uri={props.uri} prefix={props.prefix} />
      </div>
      <div className="digit-keys">
        <NumberKey text="." prefix={props.prefix} uri={props.uri} />
        <NumberKey text="0" prefix={props.prefix} uri={props.uri} />
        <NumberKey text="+/-" prefix={props.prefix} uri={props.uri} />
      </div>
    </>
  );
}

function LetterKey(props) {
  return (
    <button
      className="letter-key"
      onClick={(e) => keyClick(props.prefix + props.text, props.uri)}
    >
      {props.text}
    </button>
  );
}

function LetterKeys(props) {
  const keys = [
    ["A", "B", "C", "D", "E"],
    ["F", "G", "H", "I", "J"],
    ["K", "L", "M", "N", "O"],
    ["P", "Q", "R", "S", "T"],
    ["U", "V", "W", "X", "Y"],
    ["Z", "SP", "DEL", "/", "CLR"],
  ];
  const keyRows = keys.map((row) => (
    <div className="letter-keys">
      {row.map((k) => (
        <LetterKey text={k} prefix={props.prefix} uri={props.uri} />
      ))}
    </div>
  ));
  return keyRows;
}

function Keypad(props) {
  return (
    <div className="keypad-outter">
      <div className="keypad">
        <UpperKeys prefix={props.prefix} exec={props.exec} uri={props.uri} />
        <div className="keypad-left">
          <div>
            <LowerKeys prefix={props.prefix} uri={props.uri} />
            <NumberKeys prefix={props.prefix} uri={props.uri} />
          </div>
          <div>
            <LetterKeys prefix={props.prefix} uri={props.uri} />
          </div>
        </div>
      </div>
    </div>
  );
}

export default Keypad;
