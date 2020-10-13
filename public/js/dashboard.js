import "./firebase.js";
import gambarGrafik from "./chart.js";

//tampilan kartu yang atas
var db0 = firebase.database().ref("device/realtime");
db0.on('value', function(snapshot){
    document.getElementById('phReal').innerHTML = snapshot.val().pH;
    document.getElementById('suhuReal').innerHTML = `${snapshot.val().temperature}<sup>o</sup>`;
    document.getElementById('kekeruhanReal').innerHTML = `${snapshot.val().turbidity} cm`;
    var pHAir=snapshot.val().pH;
    var Suhu=snapshot.val().temperature;
    var Kekeruhan=snapshot.val().turbidity;
    keterangan(pHAir,Suhu, Kekeruhan);
})

function keterangan(pHAir,Suhu, Kekeruhan){
var idhtml =["pHAir","Suhu","Kekeruhan"];
var nilai =[pHAir,Suhu,Kekeruhan];
for(let i in idhtml){
var db2 = firebase.database().ref(`device/setPoint/${idhtml[i]}`)
db2.on('value', (e)=>{
    var min = (e.val().min);
    var max = (e.val().max);
    var a = parseFloat(`${nilai[i]}`);
    var x = parseFloat(min);
    var y = parseFloat(max);
    if(a<x||a>y){
        document.getElementById(`${idhtml[i]}`).innerHTML=`melewati batas`
    }
    else{
        document.getElementById(`${idhtml[i]}`).innerHTML=`aman`
    }
})
}}

var indikator = ['pH','Suhu','Kekeruhan'];
var id = ["myAreaChartph", "myAreaChartSuhu","myAreaChartKekeruhan"];

//filter data untuk 7hari
var today = new Date();
var days_before   = new Date().setDate(today.getDate()-7);
var tujuh_hari = new Date(days_before).toISOString().slice(0,10);
var days_after   = new Date().setDate(today.getDate()+1);
var sekarang = new Date(days_after).toISOString().slice(0,10);
console.log(sekarang);
var db = firebase.database().ref('device/data').orderByChild('date').startAt(`${tujuh_hari}`).endAt(`${sekarang}`);
db.once("value", function(snapshot){
    var data=[];
    for(let i in snapshot.val()){
        data.push(snapshot.val()[i]);
    }

fil(data);
console.log(data);


})

function fil(result){
    
    var ph=[], suhu=[], kekeruhan=[], tanggal=[];
    for(let i in result){
        ph.push(result[i].pH);
        suhu.push(result[i].temperature);
        kekeruhan.push(result[i].turbidity);
        var date = new Date(result[i].date);
        var a = date.getMinutes();
        var b = date.getHours();
        var d = date.getDate();
        var e = date.getMonth();
        var f = date.getFullYear();
        tanggal.push(`${d}/${e+1}/${f} ${b}:${a}`);
    }
    console.log(date);
    gambarGrafik(indikator[0], ph, tanggal, id[0]);
    gambarGrafik(indikator[2], suhu, tanggal, id[1]);
    gambarGrafik(indikator[3], kekeruhan, tanggal, id[2]);


}

//fungsi logout
document.getElementById('logout').onclick = logout;

function logout(){
    firebase.auth().signOut();
    window.location.href= '/index.html';
  }