const pn = document.querySelector("poseName");
const rx = document.querySelector("rotX");
const ry = document.querySelector("rotY");
const rz = document.querySelector("rotZ");
const ax = document.querySelector("accX");
const ay = document.querySelector("accY");
const az = document.querySelector("accZ");
const em = document.querySelector("errorMargin");
const cl = document.querySelector("html5ColorPicker");

let selectedPose = 0;
let poses = [];

function changeColor(){
    return;
}

function selectPose(){
    let p = document.querySelector("select");
    if(p.value == "NEW"){
        pn.textContext = "";
        rx.textContext = "";
        ry.textContext = "";
        rz.textContext = "";
        ax.textContext = "";
        ay.textContext = "";
        az.textContext = "";
        em.textContext = "";
    }
    if(p.value == "READ"){
    }
    alert(p.value);
}

function updatePose(){
    pose[selectedPose].name = pn.value;
    pose[selectedPose].rotX = rx.value;
    pose[selectedPose].rotY = ry.value;
    pose[selectedPose].rotZ = rz.value;
    pose[selectedPose].accX = ax.value;
    pose[selectedPose].accY = ay.value;
    pose[selectedPose].accZ = az.value;
    pose[selectedPose].errV = em.value;
    pose[selectedPose].color = cl.value;
}

function removePose(){
}

