import type { CSSProperties } from 'react'
import { useStore } from '../state/store'
import { NRPN_ROWS } from '../state/seed'
import { panelCard, sectionLabel, colHd, keyABtn, spin, segOn, segOff, segOnSm, segOffSm } from '../theme/tokens'
import { Knob } from '../components/Knob'
import { Toggle } from '../components/Toggle'

const dispChipSm: CSSProperties = { font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-ink)', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 3, padding: '2px 10px', boxShadow: 'var(--disp-inset)', textAlign: 'center' }
const rowLabel: CSSProperties = { font: "600 10px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--val)' }

export function MidiView() {
  const midi = useStore((s) => s.midi)
  const setMidiField = useStore((s) => s.setMidiField)
  const startLearn = useStore((s) => s.startLearn)
  const clearCc = useStore((s) => s.clearCc)
  const audioStub = useStore((s) => s.audioStub)

  return (
    <div style={{ display: 'grid', gridTemplateColumns: '300px 1fr 250px', gap: 10, padding: '12px 14px' }}>
      {/* left: hardware + engine + MEP4 */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: 10 }}>
        <div style={{ ...panelCard, padding: '11px 13px', display: 'flex', flexDirection: 'column', gap: 11 }}>
          <div style={sectionLabel}>HARDWARE</div>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
            <div style={rowLabel}>DEVICE Nº</div>
            <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
              <div style={dispChipSm}>{midi.device}</div>
              <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
                <div style={{ ...spin, cursor: 'pointer' }} onClick={() => setMidiField('device', Math.min(16, midi.device + 1))}>▲</div>
                <div style={{ ...spin, cursor: 'pointer' }} onClick={() => setMidiField('device', Math.max(1, midi.device - 1))}>▼</div>
              </div>
            </div>
          </div>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
            <div style={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
              <div style={rowLabel}>ECHO EDITS TO HARDWARE</div>
              <div style={{ font: "400 8.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>EDIT HERE, HEAR IT ON YOUR DX21</div>
            </div>
            <Toggle on={midi.echo} onToggle={() => setMidiField('echo', !midi.echo)} />
          </div>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
            <div style={rowLabel}>MIDI CHANNEL</div>
            <div style={dispChipSm}>{midi.channel}</div>
          </div>
        </div>

        <div style={{ ...panelCard, padding: '11px 13px', display: 'flex', flexDirection: 'column', gap: 11 }}>
          <div style={sectionLabel}>ENGINE</div>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 6 }}>
            <div style={rowLabel}>MODE</div>
            <div style={{ display: 'flex', gap: 3 }}>
              <div onClick={() => setMidiField('engine', 'VINTAGE')} style={{ ...(midi.engine === 'VINTAGE' ? segOn : segOff), flex: 1, textAlign: 'center', cursor: 'pointer' }}>VINTAGE 8-VOICE</div>
              <div onClick={() => setMidiField('engine', 'MODERN')} style={{ ...(midi.engine === 'MODERN' ? segOn : segOff), flex: 1, textAlign: 'center', cursor: 'pointer' }}>MODERN</div>
            </div>
            <div style={{ font: "400 8.5px 'IBM Plex Mono'", color: 'var(--faint)', lineHeight: 1.5 }}>8-VOICE POLY · 12-BIT DAC CHARACTER · AUTHENTIC ENVELOPE STEPPING</div>
          </div>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
            <div style={rowLabel}>OVERSAMPLING</div>
            <div style={{ display: 'flex', gap: 3 }}>
              {(['1×', '2×', '4×'] as const).map((o) => (
                <div key={o} onClick={() => setMidiField('oversample', o)} style={{ ...(midi.oversample === o ? segOnSm : segOffSm), cursor: 'pointer' }}>{o}</div>
              ))}
            </div>
          </div>
        </div>

        <div style={{ background: 'var(--inset)', border: '1px solid var(--panel-b)', borderRadius: 6 }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: 8, padding: '8px 13px', borderBottom: '1px solid var(--head-b)' }}>
            <div style={{ font: "700 12px 'Barlow Condensed'", letterSpacing: '.08em', color: 'var(--ink)' }}>MEP4</div>
            <div style={{ font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.14em', color: 'var(--silk)' }}>NOTE ECHO · BEFORE THE SYNTH</div>
            <div style={{ flex: 1 }} />
            <div style={{ width: 8, height: 8, borderRadius: '50%', background: 'var(--acc)', boxShadow: '0 0 7px var(--accg)' }} />
          </div>
          <div style={{ display: 'flex', gap: 14, padding: '11px 13px', alignItems: 'flex-start' }}>
            <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 4 }}><div style={mep4Label}>DIVISION</div><div style={dispChipSm}>1/8·</div></div>
            <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 4 }}><div style={mep4Label}>REPEATS</div><div style={dispChipSm}>4</div></div>
            <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 4 }}><div style={mep4Label}>VEL DECAY</div><Knob value={30} size={30} /><div style={{ font: "500 9.5px 'IBM Plex Mono'", color: 'var(--val)' }}>−12</div></div>
            <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 4 }}><div style={mep4Label}>TRANSPOSE</div><div style={dispChipSm}>+12</div></div>
          </div>
        </div>
      </div>

      {/* center: CC assignments */}
      <div style={{ ...panelCard, padding: '11px 13px', display: 'flex', flexDirection: 'column', gap: 9 }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
          <div style={sectionLabel}>CC ASSIGNMENTS</div>
          <div style={{ flex: 1 }} />
          <div style={{ display: 'flex', alignItems: 'center', gap: 6, background: 'var(--inset)', border: '1px solid var(--panel-b)', borderRadius: 4, padding: '4px 9px', width: 180 }}><div style={{ font: "400 10px 'IBM Plex Mono'", color: 'var(--faint)' }}>⌕ SEARCH PARAMETERS…</div></div>
          <div onClick={() => setMidiField('learnMode', !midi.learnMode)} style={{ ...keyABtn, cursor: 'pointer', opacity: midi.learnMode ? 1 : 0.6 }}>LEARN MODE {midi.learnMode ? 'ON' : 'OFF'}</div>
        </div>
        <div style={{ display: 'grid', gridTemplateColumns: '1fr 64px 70px 70px', gap: 10, padding: '0 10px' }}>
          <div style={colHd}>PARAMETER</div><div style={{ ...colHd, textAlign: 'center' }}>CC Nº</div><div style={{ ...colHd, textAlign: 'center' }}>LEARN</div><div style={{ ...colHd, textAlign: 'center' }}>CLEAR</div>
        </div>
        <div style={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
          {midi.ccRows.map((r, i) => {
            const learning = midi.learningRow === i
            const rs: CSSProperties = { display: 'grid', gridTemplateColumns: '1fr 64px 70px 70px', gap: 10, alignItems: 'center', padding: '5px 10px', borderRadius: 4, background: learning ? 'var(--szl)' : (i % 2 ? 'var(--inset)' : 'transparent'), border: '1px solid ' + (learning ? 'var(--szl-b)' : 'transparent') }
            const assigned = r.assigned
            const ccs: CSSProperties = { font: "500 11px 'IBM Plex Mono'", textAlign: 'center', padding: '2px 0', borderRadius: 3, color: assigned ? 'var(--disp-ink)' : 'var(--faint)', background: assigned ? 'var(--disp)' : 'var(--inset)', border: '1px solid ' + (assigned ? 'var(--disp-b)' : 'var(--panel-b)'), boxShadow: assigned ? 'var(--disp-inset)' : 'none' }
            const ls: CSSProperties = learning
              ? { font: "600 9px 'Barlow Condensed'", letterSpacing: '.12em', textAlign: 'center', padding: '3px 0', borderRadius: 3, color: 'var(--keyA-ink)', background: 'var(--keyA)', animation: 'blip 1.2s ease-in-out infinite', cursor: 'pointer' }
              : { font: "600 9px 'Barlow Condensed'", letterSpacing: '.12em', textAlign: 'center', padding: '3px 0', borderRadius: 3, color: 'var(--silk)', border: '1px solid var(--panel-b)', cursor: 'pointer' }
            const disabled = r.cc === '—'
            const cl: CSSProperties = { font: "600 9px 'Barlow Condensed'", letterSpacing: '.12em', textAlign: 'center', padding: '3px 0', borderRadius: 3, color: disabled ? 'var(--panel-b)' : 'var(--silk)', border: '1px solid var(--panel-b)', opacity: disabled ? 0.4 : 1, cursor: disabled ? 'default' : 'pointer' }
            return (
              <div key={i} style={rs}>
                <div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--val)' }}>{r.param}</div>
                <div style={ccs}>{r.cc}</div>
                <div style={ls} onClick={() => startLearn(i)}>{learning ? 'LISTENING…' : 'LEARN'}</div>
                <div style={cl} onClick={() => !disabled && clearCc(i)}>CLEAR</div>
              </div>
            )
          })}
        </div>
        <div style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)' }}>MOVE A CONTROL ON YOUR CONTROLLER TO BIND THE HIGHLIGHTED ROW · 118 MORE PARAMETERS</div>
      </div>

      {/* right: NRPN reference */}
      <div style={{ ...panelCard, padding: '11px 13px', display: 'flex', flexDirection: 'column', gap: 9 }}>
        <div style={{ display: 'flex', alignItems: 'baseline', justifyContent: 'space-between' }}><div style={sectionLabel}>NRPN REFERENCE</div><div style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)' }}>READ-ONLY</div></div>
        <div style={{ display: 'grid', gridTemplateColumns: '1fr 56px', gap: 8, padding: '0 4px' }}><div style={colHd}>PARAMETER</div><div style={colHd}>MSB LSB</div></div>
        <div style={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
          {NRPN_ROWS.map(([p, v], i) => (
            <div key={i} style={{ display: 'grid', gridTemplateColumns: '1fr 56px', gap: 8, padding: 4, borderBottom: '1px solid var(--panel-b)' }}><div style={{ font: "500 10.5px 'IBM Plex Mono'", color: 'var(--val)' }}>{p}</div><div style={{ font: "500 10.5px 'IBM Plex Mono'", color: 'var(--silk)' }}>{v}</div></div>
          ))}
        </div>
        <div onClick={() => audioStub('copy NRPN map')} style={{ marginTop: 'auto', padding: '8px 10px', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 4, boxShadow: 'inset 0 1px 5px rgba(0,0,0,.3)', cursor: 'pointer' }}><div style={{ font: "500 10px 'IBM Plex Mono'", color: 'var(--disp-ink)' }}>COPY FULL NRPN MAP → CLIPBOARD</div></div>
      </div>
    </div>
  )
}

const mep4Label: CSSProperties = { font: "600 8.5px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--silk)' }
