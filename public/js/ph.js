import "./firebase.js";
import gambarGrafik from "./chart.js";
import tabel from "./tabel.js";

var indikator = 'pH';
var id = "myAreaChartph";
var idtabel ="tabelph";
var min, max;
var db2 = firebase.database().ref('device/setPoint/pHAir')
db2.on('value', (e)=>{
    min = (e.val().min);
    max = (e.val().max);
})
var today = new Date();
var days_before   = new Date().setDate(today.getDate()-30);
var satu_bulan = new Date(days_before).toISOString().slice(0,10);
var days_after   = new Date().setDate(today.getDate()+1);
var sekarang = new Date(days_after).toISOString().slice(0,10);
var db = firebase.database().ref('device/data').orderByChild('date').startAt(`${satu_bulan}`).endAt(`${sekarang}`);
db.once("value", function(snapshot){
    var data=[];
    for(let i in snapshot.val()){
        data.push(snapshot.val()[i]);
    }
    min, max;
    console.log(data);
    fil(data);

})




function fil(result){
    var ph=[],tanggal=[],date, a, b, d,e,f,max0=[],min0=[];
    document.getElementById(idtabel).innerHTML="";
    for(let i in result){
        ph.push(result[i].pH);
        date = new Date(result[i].date);
        a = date.getMinutes();
        b = date.getHours();
        d = date.getDate();
        e = date.getMonth();
        f = date.getFullYear();
        tanggal.push(`${d}/${e+1}/${f} ${b}:${a}`);
        min0.push(min);
        max0.push(max);
        tabel(idtabel, `${d}/${e+1}/${f} ${b}:${a}`, result[i].pH,min,max);
    }
    gambarGrafik(indikator, ph, tanggal, id,min0,max0);
}

document.getElementById('logout').onclick = logout;

function logout(){
    firebase.auth().signOut();
    window.location.href= '/index.html';
  }