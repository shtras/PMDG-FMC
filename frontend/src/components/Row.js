import Letter from "./Letter";
import side_btn_left from "../images/side_btn_left.png";
import side_btn_left2 from "../images/side_btn_left2.png";
import side_btn_right from "../images/side_btn_right.png";
import side_btn_right2 from "../images/side_btn_right2.png";
import side_wall from "../images/side_wall.png";
import side_wall_left2 from "../images/side_wall_left2.png";
import side_wall_right2 from "../images/side_wall_right2.png";
import click from "../sounds/click.wav";

function SideButton(props) {
  function sideButtonClick(buttonId) {
    const audio = new Audio(click);
    audio.play();
    fetch(`${props.uri}/button_click`, {
      method: "POST",
      headers: {
        "Content-Type": "text/plain", // Specify the content type you are sending
      },
      body: buttonId,
    })
      .then((response) => {
        console.log(response);
      })
      .catch((e) => {
        console.log(e);
        props.setLastError(e);
      });
  }
  if (props.sideButton) {
    const buttonId = props.pos + props.row / 2;
    if (props.pos === "L") {
      return (
        <>
          <img
            src={side_btn_left}
            onClick={(e) => sideButtonClick(props.prefix + buttonId)}
            alt={"Button" + buttonId}
          />
          <img src={side_btn_left2} />
        </>
      );
    } else {
      return (
        <>
          <img src={side_btn_right2} />
          <img
            src={side_btn_right}
            onClick={(e) => sideButtonClick(props.prefix + buttonId)}
            alt={"Button" + buttonId}
          />
        </>
      );
    }
  }
  if (props.pos === "L") {
    return (
      <>
        <img src={side_wall} alt="" />
        <img src={side_wall_left2} />
      </>
    );
  } else {
    return (
      <>
        <img src={side_wall_right2} />
        <img src={side_wall} alt="" />
      </>
    );
  }
}

function Row(props) {
  const lettersList = props.letters.map((l) => (
    <Letter
      key={"" + l.id + props.id}
      text={l.text}
      small={l.small}
      color={l.color}
      reverse={l.reverse}
      unused={l.unused}
    />
  ));
  return (
    <>
      <div className="row">
        <SideButton
          pos={"L"}
          row={props.id}
          sideButton={props.sideButton}
          prefix={props.prefix}
          uri={props.uri}
        />
        {lettersList}
        <SideButton
          pos={"R"}
          row={props.id}
          sideButton={props.sideButton}
          prefix={props.prefix}
          uri={props.uri}
        />
      </div>
    </>
  );
}

export default Row;
