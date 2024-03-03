function Letter(props) {
  let t = props.text;
  if (typeof t == "number") {
    if (t === 176) {
      t = "°";
    } else if (t === 234) {
      t = "▢";
    } else if (t === 161) {
      t = "←";
    } else if (t === 162) {
      t = "→";
    } else if (t === 235) {
      t = "█";
    } else {
      t = "<<<" + t + ">>>";
    }
  }
  const classes = ["letter"];
  if (props.small) {
    classes.push("small");
  }
  const style = {
    color: props.color || "white",
  };
  if (props.reverse) {
    style.color = "black";
    style.backgroundColor = props.color || "white";
  }
  if (props.unused) {
    style.color = "darkgrey";
  }
  return (
    <div className={classes.join(" ")} style={style}>
      {t}
    </div>
  );
}

export default Letter;
