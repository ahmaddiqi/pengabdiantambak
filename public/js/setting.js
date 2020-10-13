import "./firebase.js";


var db = firebase.database().ref('device/setPoint');
document.getElementById("form").addEventListener("click", (e)=>{
    e.preventDefault();
    var variabel = document.getElementById("variabel").value;
    var max=document.getElementById("max").value;
    var min=document.getElementById("min").value;
    if(min!="" && max!="") {
        db.child(`${variabel}`).update({min});
        db.child(`${variabel}`).update({max});
    }
    else if(min!=""){
        db.child(`${variabel}`).update({min});
    }
    else if(max!=""){
        db.child(`${variabel}`).update({max});
    }
});


db.on('value', (e)=>{
    document.getElementById('tabelSetting').innerHTML="";
    for(let i in e.val()){
        document.getElementById('tabelSetting').innerHTML+=`
        <tr>
            <td>${i}</td>
            <td>${e.val()[i].min}</td>
            <td>${e.val()[i].max}</td>
        </tr>`
    }
})

var db2 = firebase.database().ref('device/1/config');
var updateTime=[];
db2.child("updateTime").on('value', (e)=>{
    updateTime=e.val();
})
document.getElementById("waktu").addEventListener("click", (e)=>{
    e.preventDefault();
    for(var i=0; i<=4;i++){
        if(document.getElementById(`jam${i+1}`).value!=""){
            updateTime[i]=document.getElementById(`jam${i+1}`).value;
            console.log(updateTime);
            db2.update({updateTime});
        }
    }
});

db2.child("updateTime").on('value', (e)=>{
    document.getElementById('tabelWaktu').innerHTML="";
    var no = 1;
    for(let i in e.val()){
        document.getElementById('tabelWaktu').innerHTML+=`
        <tr>
            <td>${no++}</td>
            <td>${e.val()[i]}</td>
        </tr>`   
    }

})
document.getElementById('logout').onclick = logout;

function logout(){
    firebase.auth().signOut();
    window.location.href= '/index.html';
  }