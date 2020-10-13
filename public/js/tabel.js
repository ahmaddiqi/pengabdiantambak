function tabel(idtabel,tanggal,nilai, min, max){
    var a = parseFloat(nilai);
    var b = parseFloat(min);
    var c = parseFloat(max);
    var keterangan, kelas;
    if(a<b||a>c){
        keterangan ="Tidak Aman";
        kelas = "kelebihan"
    }
    else{
        keterangan ="Aman";
        kelas = "aman";
    }
    document.getElementById(idtabel).innerHTML+=`
    <tr>
        <td>${tanggal}</td>
        <td>${nilai}</td>
        <td>${min}</td>
        <td>${max}</td>
        <td class="${kelas}">${keterangan}</td>
    </tr>`
}
export default tabel;